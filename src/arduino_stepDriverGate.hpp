#ifndef ARDUINO_STEPDRIVERGATE_HPP
#define ARDUINO_STEPDRIVERGATE_HPP

#include <Arduino.h>

class Arduino_StepDriverGate 
    {
        
    private:
        byte mAnalogPin;
        byte mStepPin;
        byte mDirPin;
        byte mEnablePin;
        uint16_t mMaxSteps = 200; // Steps per Revolution
        int16_t mAvgSamples[20] = {0}; // for averaging the analog input
        byte mAvgIndex = 0;

        uint16_t mCurrentStep = 0; // Current Step Position
        uint16_t mTargetStep = 0; // Target Step Position
        uint16_t mFrequency = 50; //Hz
        uint16_t mHighTime = 20; //micro seconds
        uint16_t mLowTime = 20; //micro seconds
        float mVoltage ;
        float mVoltageStepResolution; // Steps derived from Voltage

        float mAnalogMin = 0.0; // Min Value in mV
        float mAnalogMax = 5000.0; // Max Value  in mV
        float mADCMin = 0.0;
        float mADCMax = 1023.0;

        bool mPulseSignalActive = false; 
        bool mPulseSignalInactive = false;
        bool mPauseWasActive = false;
        bool mDirectionChanged = false; // used for direction change detection
        bool mPulseInverted = false;
        bool mSysInitDone = false; 
        bool mPositionInit = true; 

        uint32_t mPerfomanceStart;
        uint32_t mPerformanceEnd;

        uint32_t mTimestampMicrosStepPulse;
        uint32_t mTimestampMicrosHigh;
        uint32_t mTimestampMicrosLow;

        struct outputs 
        {
            bool Sig = false;
            bool en = true; // default enabled
            bool dir = false;
        } mOut;


    public:
        Arduino_StepDriverGate(byte analogPin, byte stepPin, byte dirPin, byte enPin, uint16_t maxSteps) 
        : mAnalogPin(analogPin), mStepPin(stepPin), mDirPin(dirPin), mEnablePin(enPin), mMaxSteps(maxSteps) {}

        enum class direction
        {
            NORMAL,
            INVERTED
        };
        enum class PulseStep
        {
            ACTIVE,
            INACTIVE,
            PAUSE

        } mPulseStepState = PulseStep::ACTIVE;

        auto setFrequency(const uint16_t frequency) -> void;
        auto setAnalogMin(const uint16_t analogMin) -> void {mAnalogMin = analogMin;};
        auto setAnalogMax(const uint16_t analogMax) -> void {mAnalogMax = analogMax;};
        auto setEnable(const bool enable) -> void {mOut.en = enable;};
        auto setInvertPulse(const bool invertPulse) -> void {mPulseInverted = invertPulse;};
        auto setHighTime(const uint16_t highTime) -> void {mHighTime = highTime;};
        auto setLowTime(const uint16_t lowTime) -> void {mLowTime = lowTime;};
        auto setADC_min(const uint16_t adcMin) -> void {mADCMin = adcMin;};
        auto setADC_max(const uint16_t adcMax) -> void {mADCMax = adcMax;};

        auto getPerformance() -> uint32_t ;
        auto getCurrentPosition() -> uint16_t {return mCurrentStep;};

        auto run() -> void;
        auto stop() -> void;

        private:
        auto handle(const uint16_t _targetStep) -> void;
        auto fmap(float x, float in_min, float in_max, float out_min, float out_max) -> float
        {
            return out_min + (x - in_min) * (out_max - out_min) / (in_max - in_min);
        }
        auto setOutputs() -> void;

        auto createSignal(direction _dir) -> void;
};

#endif // ARDUINO_STEPDRIVERGATE_HPP