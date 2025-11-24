#include "arduino_stepDriverGate.hpp"

auto Arduino_StepDriverGate::run() -> void
{
    mPerfomanceStart = micros();
    static auto init = false;
    if (!init)
    {
        pinMode(mAnalogPin, INPUT);
        pinMode(mStepPin, OUTPUT);
        pinMode(mDirPin, OUTPUT);
        pinMode(mEnablePin, OUTPUT);
        init = true;

        mCurrentStep = mMaxSteps; // start at max position for position init
    }

    // calculate target step from input voltage
    auto raw = analogRead(mAnalogPin);
    mVoltage = fmap(raw, 0.0f, 1023.0f, 0.0f, 5000.0f);
    mVoltageStepResolution = (mAnalogMax - mAnalogMin) / mMaxSteps;

    // Force to 0 position at startup
    if(mPositionInit)
    {
        mVoltage = 0.0f; // force to 0 position at startup

        if(mCurrentStep <= 0)
        {
            mPositionInit = false; // position initialized
        }
    }

    mTargetStep = static_cast<uint16_t>((mVoltage - mAnalogMin) / mVoltageStepResolution);

    if (mVoltage >= mAnalogMin && mVoltage <= mAnalogMax)
    {
        handle(mTargetStep);
    }else 
    {
        handle(0);
    }
    mPerformanceEnd = micros();
}

/**
 * @brief public method to set the stepper motor frequency for the step pulse (Hz)
 * 
 * @param frequency - Hz - see datasheet of stepper motor
 */
auto Arduino_StepDriverGate::setFrequency(uint16_t frequency) -> void
{
    if (frequency < 1)
        mFrequency = 1;

    mFrequency = frequency;
}


/**
 * @brief create a step pulse for the stepper motor
 *
 */
auto Arduino_StepDriverGate::handle(const uint16_t _targetStep) -> void
{

    if (mCurrentStep < _targetStep)
    {
        createSignal(direction::NORMAL);
    }
    else if ( (mCurrentStep > _targetStep) )
    {
        createSignal(direction::INVERTED);
    }

    setOutputs();

}
auto Arduino_StepDriverGate::createSignal(direction _dir) -> void
{
    auto timestamp = micros();
    static auto timestampMicrosStepPulse = timestamp;
    static auto timestampMicrosHigh = timestamp;
    static auto timestampMicrosLow = timestamp;
    auto period = 1000000 / mFrequency; // in microseconds

    // Trigger step
    mPauseWasActive = timestamp >= timestampMicrosLow + mLowTime;


    // If drirection changed, set a pause for stabilization
    bool directionNow = (_dir == direction::INVERTED) ? true : false;
    if (mDirectionChanged != directionNow)
    {
        mPulseStepState = PulseStep::ACTIVE;
        mDirectionChanged = directionNow;
        timestampMicrosLow = timestamp;
        timestampMicrosStepPulse = timestamp;
        timestampMicrosHigh = timestamp;
    }

    switch (_dir)
    {
    case direction::NORMAL:
        mOut.dir = false; // CW
        break;
    case direction::INVERTED:
        mOut.dir = true; // CCW
        break;
    }

    switch (mPulseStepState)
    {
    case PulseStep::ACTIVE:

        mPulseSignalActive = timestamp >= timestampMicrosStepPulse + period;

        if (mPulseSignalActive)
        {
            timestampMicrosStepPulse = timestamp;
            timestampMicrosLow = timestamp;

            mOut.Sig = true;
            mPulseStepState = PulseStep::INACTIVE;
        }
        break;

    case PulseStep::INACTIVE:

        mPulseSignalInactive = timestamp >= timestampMicrosHigh + mHighTime;

        if (mPulseSignalInactive)
        {

            timestampMicrosStepPulse = timestamp;
            timestampMicrosHigh = timestamp;

            mOut.Sig = false;

            mPulseStepState = PulseStep::PAUSE;
        }
        break;
    case PulseStep::PAUSE:
        if (mPauseWasActive)
        {
            timestampMicrosLow = timestamp;

            switch (_dir)
            {
            case direction::NORMAL:
                mCurrentStep++;
                break;
            case direction::INVERTED:
                mCurrentStep--;
                break;
            }

            mPulseStepState = PulseStep::ACTIVE;
        }
        break;
    }
}

/**
 * @brief Stops the stepper motor by disabling the driver immediately.
 * @brief The stepper will stop moving and hold its current position.
 */
auto Arduino_StepDriverGate::stop() -> void
{
    // Disable Stepper Driver
    mOut.en = false;
    mOut.Sig = false;
}

auto Arduino_StepDriverGate::setOutputs() -> void
{
    digitalWrite(mStepPin, mOut.Sig);
    digitalWrite(mDirPin, mOut.dir);
    digitalWrite(mEnablePin, mOut.en);
}
/**
 * @brief Function to get the performance time of the last run loop in microseconds
 * @brief this is useful for checking the processing time in realtime applications
 * @return uint32_t - time in microseconds
 */
auto Arduino_StepDriverGate::getPerformance() -> uint32_t 
{
    return mPerformanceEnd - mPerfomanceStart;
}
