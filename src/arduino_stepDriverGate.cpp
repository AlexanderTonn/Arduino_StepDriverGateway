#include "arduino_stepDriverGate.hpp"

auto Arduino_StepDriverGate::run() -> void
{
    mPerfomanceStart = micros();

    // ------------------------------------------------------------
    // Hardware initialization
    // ------------------------------------------------------------
    if (!mSysInitDone)
    {
        pinMode(mStepPin, OUTPUT);
        pinMode(mDirPin, OUTPUT);
        pinMode(mEnablePin, OUTPUT);

        if(mInitDirection == direction::NORMAL)
        {
            mCurrentStep = mMaxSteps; // Assume valve is at max position
        }
        if(mInitDirection == direction::INVERTED)
        {
            mCurrentStep = 0; // Assume valve is at min position
        }

        

        mSysInitDone = true;
    }

    // ------------------------------------------------------------
    // Determine input value
    // ------------------------------------------------------------
    if (mAnalogPin != NO_ANALOG_PIN)
    {
        constexpr size_t sampleCount =
            sizeof(mAvgSamples) / sizeof(mAvgSamples[0]);

        // Add new ADC sample
        mAvgSamples[mAvgIndex++] = analogRead(mAnalogPin);

        // Calculate average when buffer is full
        if (mAvgIndex >= sampleCount)
        {
            mAvgIndex = 0;

            uint32_t raw = 0;

            for (size_t i = 0; i < sampleCount; ++i)
            {
                raw += mAvgSamples[i];
            }

            const float avg =
                static_cast<float>(raw) /
                static_cast<float>(sampleCount);

            // Convert ADC value into configured analog range
            mVoltage = fmap(
                avg,
                mADCMin,
                mADCMax,
                mAnalogMin,
                mAnalogMax);
        }
    }
    else
    {
        // Software input already represents the configured
        // engineering range, e.g. 0...100 %
        mVoltage = static_cast<float>(mSoftwareInput);
    }

    // ------------------------------------------------------------
    // Position initialization
    // ------------------------------------------------------------
    if (mPositionInit)
    {
        // Force valve towards minimum position
        switch (mInitDirection)
        {
        case direction::NORMAL:
            mVoltage = mAnalogMin;

            if (mCurrentStep == 0)
            {
                mPositionInit = false;
            }
            break;
        case direction::INVERTED:
            mVoltage = mAnalogMax;

            if(mCurrentStep == mMaxSteps)
            {
                mPositionInit = false; 
            }

        default:
            break;
        }
    }

    // ------------------------------------------------------------
    // Limit input value
    // ------------------------------------------------------------
    if (mVoltage < mAnalogMin)
    {
        mVoltage = mAnalogMin;
    }
    else if (mVoltage > mAnalogMax)
    {
        mVoltage = mAnalogMax;
    }

    // ------------------------------------------------------------
    // Calculate target position
    // ------------------------------------------------------------
    const float analogRange = mAnalogMax - mAnalogMin;

    if ((analogRange > 0.0f) && (mMaxSteps > 0))
    {
        mVoltageStepResolution =
            analogRange / static_cast<float>(mMaxSteps);

        mTargetStep = static_cast<uint16_t>(
            (mVoltage - mAnalogMin) /
            mVoltageStepResolution);

        // Protect against floating point rounding
        if (mTargetStep > mMaxSteps)
        {
            mTargetStep = mMaxSteps;
        }
    }
    else
    {
        mVoltageStepResolution = 0.0f;
        mTargetStep = 0;
    }

    // ------------------------------------------------------------
    // Move stepper towards target
    // ------------------------------------------------------------
    handle(mTargetStep);

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
    const uint32_t now = micros();
    const uint32_t period = 1000000UL / mFrequency;

    mOut.dir = (_dir == direction::INVERTED);

    if (mOut.Sig)
    {
        // End HIGH pulse
        if ((uint32_t)(now - mTimestampMicrosHigh) >= mHighTime)
        {
            mOut.Sig = false;
        }
    }
    else
    {
        // Start next STEP
        if ((uint32_t)(now - mTimestampMicrosStepPulse) >= period)
        {
            mTimestampMicrosStepPulse = now;
            mTimestampMicrosHigh = now;

            mOut.Sig = true;

            if (_dir == direction::NORMAL)
            {
                if (mCurrentStep < mMaxSteps)
                    ++mCurrentStep;
            }
            else
            {
                if (mCurrentStep > 0)
                    --mCurrentStep;
            }
        }
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
