/*
  ==============================================================================

    WDF_Filters.h
    Created: 19 Oct 2023 10:07:40am
    Author:  mitchschneider_

  ==============================================================================
*/

#pragma once
#include "fxobjects_Schneider.cpp"

class SchneiderWDF_LPF : public IAudioSignalProcessor
{
public:
    SchneiderWDF_LPF(void) { createWDF(); }
    ~SchneiderWDF_LPF(void) {}

    virtual bool reset(double _sampleRate)
    {
        sampleRate = _sampleRate;

        seriesTerminatedAdaptor_C.reset(_sampleRate);
        seriesTerminatedAdaptor_C.initializeAdaptorChain();
        return true;
    }

    virtual bool canProcessAudioFrame() { return false; }

    virtual double processAudioSample(double xn)
    {
        // --- push audio sample into series L1
        seriesTerminatedAdaptor_C.setInput1(xn);

        // --- output is at terminated L2's output2
        return seriesTerminatedAdaptor_C.getOutput1();

    }

    void createWDF()
    {
        //     initial values for fc = 1kHz
        //     C = 0.0000007073585179619
        
        seriesTerminatedAdaptor_C.setComponent(wdfComponent::C, 0.0000007073585179619);
        seriesTerminatedAdaptor_C.setSourceResistance(600.0); // --- Rs = 600

    }

    WDFParameters getParameters()
    {
        return wdfParameters;
    }

    void setParameters(const WDFParameters& _wdfParameters)
    {
        if (_wdfParameters.fc != wdfParameters.fc ||
            _wdfParameters.Q != wdfParameters.Q ||
            _wdfParameters.boostCut_dB != wdfParameters.boostCut_dB ||
            _wdfParameters.frequencyWarping != wdfParameters.frequencyWarping)
        {
            wdfParameters = _wdfParameters;
            double fc_Hz = wdfParameters.fc;

            if (wdfParameters.frequencyWarping)
            {
                double arg = (kPi*fc_Hz) / sampleRate;
                fc_Hz = fc_Hz*(tan(arg) / arg);
            }
            
            double capacitorValue = 1 / (2*kPi*fc_Hz*2.251131e2);
            DBG("capacitor value is"+std::to_string(capacitorValue));

            seriesTerminatedAdaptor_C.setComponentValue(capacitorValue);
            seriesTerminatedAdaptor_C.initializeAdaptorChain();
        }
    }

protected:
    WDFParameters wdfParameters;
    WdfSeriesTerminatedAdaptor seriesTerminatedAdaptor_C;

    double sampleRate = 1.0;

};


class SchneiderWDF_HPF : public IAudioSignalProcessor
{
public:
    SchneiderWDF_HPF(void)
    {
        createWDF();
    }
    
    ~SchneiderWDF_HPF(void) {}

    virtual bool reset(double _sampleRate)
    {
        sampleRate = _sampleRate;
        parallelTerminatedAdaptor_C.reset(_sampleRate);
        parallelTerminatedAdaptor_C.initializeAdaptorChain();
        return true;
    }

    virtual bool canProcessAudioFrame() { return false; }

    virtual double processAudioSample(double xn)
    {
        parallelTerminatedAdaptor_C.setInput1(xn);
        return parallelTerminatedAdaptor_C.getOutput1();
    }

    void createWDF()
    {
        //     initial values for fc = 1kHz
        //     C = 0.0000007073585179619
        
        parallelTerminatedAdaptor_C.setComponent(wdfComponent::C, 0.0000007073585179619);
        parallelTerminatedAdaptor_C.setSourceResistance(600.0); // --- Rs = 600
    }

    WDFParameters getParameters()
    {
        return wdfParameters;
    }

    void setParameters(const WDFParameters& _wdfParameters)
    {
        if (_wdfParameters.fc != wdfParameters.fc ||
            _wdfParameters.Q != wdfParameters.Q ||
            _wdfParameters.boostCut_dB != wdfParameters.boostCut_dB ||
            _wdfParameters.frequencyWarping != wdfParameters.frequencyWarping)
        {
            wdfParameters = _wdfParameters;
            double fc_Hz = wdfParameters.fc;

            if (wdfParameters.frequencyWarping)
            {
                double arg = (kPi*fc_Hz) / sampleRate;
                fc_Hz = fc_Hz*(tan(arg) / arg);
            }
            
            double capacitorValue = 1 / (2*kPi*fc_Hz*2.251131e2);
            
            parallelTerminatedAdaptor_C.setComponentValue(capacitorValue);
            parallelTerminatedAdaptor_C.initializeAdaptorChain();
        }
    }

protected:
    WDFParameters wdfParameters;
    WdfParallelTerminatedAdaptor parallelTerminatedAdaptor_C;
    
    double sampleRate = 1.0;

};
