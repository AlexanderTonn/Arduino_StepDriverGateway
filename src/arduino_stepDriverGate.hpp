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

        uint16_t mCurrentStep = 0; // Current Step Position
        uint16_t mTargetStep = 0; // Target Step Position
        uint16_t mFrequency = 50; //Hz
        uint16_t mHighTime = 20; //micro seconds
        uint16_t mLowTime = 20; //micro seconds
        float mVoltage = 0.0f;
        uint16_t mVoltageStepResolution = 0; // Steps derived from Voltage

        float mAnalogMin = 500.0; // Min Value in mV
        float mAnalogMax = 5000.0; // Max Value  in mV

        bool mPulseSignalActive = false; 
        bool mPulseSignalInactive = false;
        bool mPauseWasActive = false;
        bool mDirectionChanged = false; // used for direction change detection
        bool mPulseInverted = false;
        bool mPositionInit = true; 

        uint32_t mPerfomanceStart;
        uint32_t mPerformanceEnd;

        struct outputs 
        {
            bool Sig;
            bool en;
            bool dir;
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

        } mPulseStepState;

        auto setFrequency(const uint16_t frequency) -> void;
        auto setAnalogMin(const uint16_t analogMin) -> void {mAnalogMin = analogMin;};
        auto setAnalogMax(const uint16_t analogMax) -> void {mAnalogMax = analogMax;};
        auto setEnable(const bool enable) -> void {mOut.en = enable;};
        auto setInvertPulse(const bool invertPulse) -> void {mPulseInverted = invertPulse;};
        auto setHighTime(const uint16_t highTime) -> void {mHighTime = highTime;};
        auto setLowTime(const uint16_t lowTime) -> void {mLowTime = lowTime;};

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