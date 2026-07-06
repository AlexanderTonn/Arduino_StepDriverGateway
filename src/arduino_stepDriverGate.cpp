#include "arduino_stepDriverGate.hpp"

auto Arduino_StepDriverGate::run() -> void
{
    mPerfomanceStart = micros();
    if (!mSysInitDone)
    {
        pinMode(mStepPin, OUTPUT);
        pinMode(mDirPin, OUTPUT);
        pinMode(mEnablePin, OUTPUT);
        mSysInitDone = true;

        mCurrentStep = mMaxSteps; // start at max position for position init
    }

    // calculate target step from input voltage
    if (mAnalogPin != NO_ANALOG_PIN)
    {
        if (mAvgIndex < ((sizeof(mAvgSamples) / 2) - 1))
        {
            mAvgSamples[mAvgIndex++] = analogRead(mAnalogPin);
        }
        else
        {
            mAvgIndex = 0;
            auto raw = 0;
            // Create Average
            for (byte i = 0; i < (sizeof(mAvgSamples) / 2) - 1; i++)
            {
                raw += mAvgSamples[i];
            }
            auto avg = raw / (sizeof(mAvgSamples) / 2);

            mVoltage = fmap(avg, mADCMin, mADCMax, mAnalogMin, mAnalogMax);
        }
    }
    else
    {
        mVoltage = fmap(mSoftwareInput, mADCMin, mADCMax, mAnalogMin, mAnalogMax);
    }

    mVoltageStepResolution = (mAnalogMax - mAnalogMin) / mMaxSteps;

    // Force to 0 position at startup
    if (mPositionInit)
    {
        mVoltage = 0.0f; // force to 0 position at startup

        if (mCurrentStep <= 0)
        {
            mPositionInit = false; // position initialized
        }
    }

    mTargetStep = static_cast<uint16_t>((mVoltage - mAnalogMin) / mVoltageStepResolution);

    if (mVoltage >= mAnalogMin && mVoltage <= mAnalogMax)
    {
        handle(mTargetStep);
    }
    else
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
    mFrequency = (frequency < 1) ? 1 : frequency;
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
    else if ((mCurrentStep > _targetStep))
    {
        createSignal(direction::INVERTED);
    }

    setOutputs();
}
auto Arduino_StepDriverGate::createSignal(direction _dir) -> void
{
    auto timestamp = micros();
    auto period = 1000000UL / mFrequency; // in microseconds

    // Trigger step
    mPauseWasActive = timestamp >= mTimestampMicrosLow + mLowTime;

    // If drirection changed, set a pause for stabilization
    bool directionNow = (_dir == direction::INVERTED) ? true : false;
    if (mDirectionChanged != directionNow)
    {
        mPulseStepState = PulseStep::ACTIVE;
        mDirectionChanged = directionNow;
        mTimestampMicrosLow = timestamp;
        mTimestampMicrosStepPulse = timestamp;
        mTimestampMicrosHigh = timestamp;
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

        mPulseSignalActive = timestamp >= mTimestampMicrosStepPulse + period;

        if (mPulseSignalActive)
        {
            mTimestampMicrosStepPulse = timestamp;
            mTimestampMicrosLow = timestamp;

            mOut.Sig = true;
            mPulseStepState = PulseStep::INACTIVE;
        }
        break;

    case PulseStep::INACTIVE:

        mPulseSignalInactive = timestamp >= mTimestampMicrosHigh + mHighTime;

        if (mPulseSignalInactive)
        {

            mTimestampMicrosStepPulse = timestamp;
            mTimestampMicrosHigh = timestamp;

            mOut.Sig = false;

            mPulseStepState = PulseStep::PAUSE;
        }
        break;
    case PulseStep::PAUSE:
        if (mPauseWasActive)
        {
            mTimestampMicrosLow = timestamp;

            if (_dir == direction::NORMAL)
            {
                if (mCurrentStep < mMaxSteps)
                    mCurrentStep++;
            }
            else
            {
                if (mCurrentStep > 0)
                    mCurrentStep--;
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
