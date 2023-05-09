/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */
#include <arduino.h>
#include "vt_cs_calibrate.h"
#include "vt_cs_fft.h"
#include "vt_cs_raw_signature_read.h"
#include "vt_debug.h"
#include <math.h>

typedef struct SPECTOGRAM_STRUCT
{
    /* Spectogram Frequency Value*/
    VT_FLOAT frequency;

    /* Spectogram Frequency Amplitude */
    VT_FLOAT magnitude;

} SPECTOGRAM;

static VT_FLOAT get_calib_range_freq(VT_UINT range_id)
{
    float exp_divisor=powf(((float)VT_CS_ADC_MAX_SAMPLING_FREQ/VT_CS_ADC_MIN_SAMPLING_FREQ),((float)1/((float)(VT_CS_MAX_SIGNATURES-1))));

        #if VT_LOG_LEVEL > 3
            VT_INT decimal;
            VT_FLOAT frac_float;
            VT_INT frac;
            decimal    = exp_divisor;
            frac_float = exp_divisor - (VT_FLOAT)decimal;
            frac       = fabsf(frac_float) * 10000;
            VTLogDebugNoTag(" \n exp_divisor %d.%04d : \n", decimal, frac);
        #endif /* VT_LOG_LEVEL > 2 */
        
    float sampling_freq=((float)VT_CS_ADC_MAX_SAMPLING_FREQ)/powf(exp_divisor,range_id);

    return sampling_freq;
}

static VT_VOID swap_spectogram_object(SPECTOGRAM* spectogram_object, VT_INT index1, VT_INT index2)
{
    float frequency_temp                = spectogram_object[index1].frequency;
    float frequency_magnitude_temp      = spectogram_object[index1].magnitude;
    spectogram_object[index1].frequency = spectogram_object[index2].frequency;
    spectogram_object[index1].magnitude = spectogram_object[index2].magnitude;
    spectogram_object[index2].frequency = frequency_temp;
    spectogram_object[index2].magnitude = frequency_magnitude_temp;
}

static VT_VOID sort_spectogram_frequency(SPECTOGRAM* spectogram_object, VT_INT start_index, VT_INT samples)
{
    VT_INT min_idx = 0;
    for (VT_INT iter1 = start_index; iter1 < ((samples - 1) + start_index); iter1++)
    {
        min_idx = iter1;
        for (VT_INT iter2 = iter1 + 1; iter2 < (samples + start_index); iter2++)
        {
            if (spectogram_object[iter2].frequency < spectogram_object[min_idx].frequency)
            {
                min_idx = iter2;
            }
        }
        swap_spectogram_object(spectogram_object, min_idx, iter1);
    }
}

static VT_VOID sort_spectogram_amplitude(SPECTOGRAM* spectogram_object, VT_INT samples)
{
    VT_INT max_idx = 0;
    for (VT_INT iter1 = 0; iter1 < (samples - 1); iter1++)
    {
        max_idx = iter1;
        for (VT_INT iter2 = iter1 + 1; iter2 < samples; iter2++)
        {
            if (spectogram_object[iter2].magnitude > spectogram_object[max_idx].magnitude)
            {
                max_idx = iter2;
            }
        }
        swap_spectogram_object(spectogram_object, max_idx, iter1);
    }
}

static VT_VOID remove_harmonics(SPECTOGRAM* spectogram_object, VT_INT start_index, VT_INT samples, VT_FLOAT sampling_frequency)
{
    VT_FLOAT frequency_multiplier    = 0;
    VT_FLOAT frequency_difference    = 0;
    VT_FLOAT frequency_allowed_delta = 0;
    sort_spectogram_frequency(spectogram_object, start_index, VT_CS_MAX_TEST_FREQUENCIES);

    for (VT_INT iter1 = start_index; iter1 < (samples + start_index - 1); iter1++)
    {
        if (spectogram_object[iter1].frequency == 0)
        {
            continue;
        }
        for (VT_INT iter2 = iter1 + 1; iter2 < (samples + start_index); iter2++)
        {
            if (spectogram_object[iter2].frequency == 0)
            {
                continue;
            }
            frequency_multiplier = spectogram_object[iter2].frequency / spectogram_object[iter1].frequency;
            if (frequency_multiplier > VT_CS_MAX_HARMONICS_REMOVAL)
            {
                break;
            }
            frequency_difference = fabsf(((VT_FLOAT)round(frequency_multiplier)) - frequency_multiplier);
            frequency_allowed_delta =
                ((sampling_frequency / (VT_FLOAT)VT_CS_SAMPLE_LENGTH) * ((VT_FLOAT)round(frequency_multiplier))) /
                spectogram_object[iter1].frequency;
            if (frequency_difference < frequency_allowed_delta)
            {
                spectogram_object[iter1].magnitude += spectogram_object[iter2].magnitude;
                spectogram_object[iter2].magnitude = 0;
            }
        }
    }
    for (VT_INT iter2 = start_index; iter2 < (samples + start_index); iter2++)
    {
        if (spectogram_object[iter2].magnitude < 1)
        {
            spectogram_object[iter2].magnitude = 0;
        }
    }
}

static VT_VOID calculate_top_N_signal_frequencies(
    SPECTOGRAM* spectogram_object, VT_INT start_index, COMPLEX* signal, VT_FLOAT sampling_frequency)
{
#if VT_LOG_LEVEL > 3
    VT_INT decimal;
    VT_FLOAT frac_float;
    VT_INT frac;
#endif /* VT_LOG_LEVEL > 2 */

    VTLogDebug("Current Signature Raw: \r\n");
#if VT_LOG_LEVEL > 3
    for (VT_INT iter = 0; iter < VT_CS_SAMPLE_LENGTH; iter++)
    {
        decimal    = signal[iter].real;
        frac_float = signal[iter].real - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d, ", decimal, frac);
    }
#endif /* VT_LOG_LEVEL > 2 */
    VTLogDebugNoTag("\r\n");

    cs_fft_dc_removal(signal, VT_CS_SAMPLE_LENGTH);
    cs_fft_windowing(signal, VT_CS_SAMPLE_LENGTH, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    cs_fft_compute(signal, VT_CS_SAMPLE_LENGTH);
    cs_fft_complex_to_magnitude(signal, VT_CS_SAMPLE_LENGTH);
    

    VTLogDebug("FFT: \r\n");
#if VT_LOG_LEVEL > 3
    for (VT_INT iter = 0; iter < VT_CS_SAMPLE_LENGTH / 2; iter++)
    {
        decimal    = signal[iter].real;
        frac_float = signal[iter].real - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d, ", decimal, frac);
    }
#endif /* VT_LOG_LEVEL > 2 */
    VTLogDebugNoTag("\r\n");

    signal[0].real = 0;
    cs_fft_normalize(signal, VT_CS_SAMPLE_LENGTH);

    VTLogDebug("Normalized FFT: \r\n");
#if VT_LOG_LEVEL > 3
    for (VT_INT iter = 0; iter < VT_CS_SAMPLE_LENGTH / 2; iter++)
    {
        decimal    = signal[iter].real;
        frac_float = signal[iter].real - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d, ", decimal, frac);
    }
#endif /* VT_LOG_LEVEL > 2 */
    VTLogDebugNoTag("\r\n");

    VT_INT peak_index                  = 0;
    VT_FLOAT frequency                 = 0;
    VT_FLOAT frequency_magnitude       = 0;
    VT_FLOAT average_of_peak_neighbour = 0;
    for (VT_INT iter = 0; iter < VT_CS_MAX_TEST_FREQUENCIES; iter++)
    {
        cs_fft_major_peak(signal, VT_CS_SAMPLE_LENGTH, sampling_frequency, &frequency, &frequency_magnitude, &peak_index);
        spectogram_object[start_index + iter].frequency = frequency;
        spectogram_object[start_index + iter].magnitude = frequency_magnitude;
        if (peak_index == 0)
        {
            average_of_peak_neighbour   = (signal[peak_index].real + signal[peak_index + 1].real) / 2.0f;
            signal[peak_index].real     = average_of_peak_neighbour;
            signal[peak_index + 1].real = average_of_peak_neighbour;
        }
        else if (peak_index == (VT_CS_SAMPLE_LENGTH - 1))
        {
            average_of_peak_neighbour   = (signal[peak_index].real + signal[peak_index - 1].real) / 2.0f;
            signal[peak_index].real     = average_of_peak_neighbour;
            signal[peak_index - 1].real = average_of_peak_neighbour;
        }
        else
        {
            average_of_peak_neighbour =
                (signal[peak_index - 1].real + signal[peak_index].real + signal[peak_index + 1].real) / 3.0f;
            signal[peak_index - 1].real = average_of_peak_neighbour;
            signal[peak_index].real     = average_of_peak_neighbour;
            signal[peak_index + 1].real = average_of_peak_neighbour;
        }
    }

        VTLogDebug("Signal after major peak: \r\n");
#if VT_LOG_LEVEL > 3
    for (VT_INT iter = 0; iter < VT_CS_SAMPLE_LENGTH / 2; iter++)
    {
        decimal    = signal[iter].real;
        frac_float = signal[iter].real - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d, ", decimal, frac);
    }
#endif /* VT_LOG_LEVEL > 2 */

    VTLogDebug("Test Frequencies: \r\n");
#if VT_LOG_LEVEL > 3
    for (VT_INT iter = 0; iter < VT_CS_MAX_TEST_FREQUENCIES; iter++)
    {
        decimal    = spectogram_object[start_index + iter].frequency;
        frac_float = spectogram_object[start_index + iter].frequency - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d : ", decimal, frac);
        decimal    = spectogram_object[start_index + iter].magnitude;
        frac_float = spectogram_object[start_index + iter].magnitude - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d \r\n", decimal, frac);
    }
#endif /* VT_LOG_LEVEL > 2 */

    remove_harmonics(spectogram_object, start_index, VT_CS_MAX_TEST_FREQUENCIES, sampling_frequency);

    VTLogDebug("Test Frequencies after Harmonic Removal: \r\n");
#if VT_LOG_LEVEL > 3
    for (VT_INT iter = 0; iter < VT_CS_MAX_TEST_FREQUENCIES; iter++)
    {
        decimal    = spectogram_object[start_index + iter].frequency;
        frac_float = spectogram_object[start_index + iter].frequency - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d : ", decimal, frac);
        decimal    = spectogram_object[start_index + iter].magnitude;
        frac_float = spectogram_object[start_index + iter].magnitude - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d \r\n", decimal, frac);
    }
#endif /* VT_LOG_LEVEL > 2 */
}

static VT_FLOAT get_raw_signature_sample_freq(VT_FLOAT signal_freq)
{
    VT_FLOAT sample_freq = signal_freq * (VT_CS_SAMPLE_LENGTH / VT_CS_CALIB_MINIMUM_CYCLES);
    if (sample_freq > VT_CS_ADC_MAX_SAMPLING_FREQ)
    {
        sample_freq = VT_CS_ADC_MAX_SAMPLING_FREQ;
    }
    return sample_freq;
}

VT_VOID cs_calibrate_repeating_signatures_compute_sampling_frequencies(VT_CURRENTSENSE_OBJECT* cs_object,
    VT_FLOAT* sampling_frequencies,
    VT_UINT sampling_frequencies_buffer_length,
    VT_UINT* num_sampling_frequencies)
{
    //VT_UINT8 fft_ranges       =   calculate_fft_ranges();
    *num_sampling_frequencies = 0;
    for (VT_UINT iter = 0; iter < VT_CS_NUM_EXP_RANGES ; iter++)
    {
        if (iter == sampling_frequencies_buffer_length)
        {
            break;
        }
        sampling_frequencies[iter] = get_calib_range_freq(iter);
        *num_sampling_frequencies  = *num_sampling_frequencies + 1;
        #if VT_LOG_LEVEL > 3
            VT_INT decimal;
            VT_FLOAT frac_float;
            VT_INT frac;
            decimal    = sampling_frequencies[iter];
            frac_float = sampling_frequencies[iter] - (VT_FLOAT)decimal;
            frac       = fabsf(frac_float) * 10000;
            VTLogDebugNoTag("%d.%04d : ", decimal, frac);
        #endif /* VT_LOG_LEVEL > 2 */
    }

    #if VT_LOG_LEVEL > 3
    Serial.printf("Number of Sampling freq : %d \n",*num_sampling_frequencies);
    #endif /* VT_LOG_LEVEL > 2 */

}

VT_VOID cs_calibrate_repeating_signatures_compute_collection_settings(
    VT_CURRENTSENSE_OBJECT* cs_object, VT_FLOAT* top_N_sample_frequencies, VT_FLOAT* lowest_sample_freq)
{
#if VT_LOG_LEVEL > 3
    VT_INT decimal;
    VT_FLOAT frac_float;
    VT_INT frac;
#endif /* VT_LOG_LEVEL > 2 */

    VTLogInfo("\tComputing Currentsense Collection Settings\n");
    SPECTOGRAM spectogram_calib[VT_CS_MAX_TEST_FREQUENCIES];
    for (VT_UINT iter = 0; iter < VT_CS_MAX_TEST_FREQUENCIES; iter++)
    {
        spectogram_calib[iter].magnitude = 0;
        spectogram_calib[iter].frequency = 0;
    }
    VT_UINT calib_ranges                          = VT_CS_NUM_EXP_RANGES;

    static VT_FLOAT adc_read_signal[VT_CS_SAMPLE_LENGTH];
    static COMPLEX signal[VT_CS_SAMPLE_LENGTH];
    memset(adc_read_signal,0,sizeof(VT_FLOAT)*VT_CS_SAMPLE_LENGTH);
    memset(signal,0,sizeof(COMPLEX)*VT_CS_SAMPLE_LENGTH);

    for (VT_UINT iter = 0; iter < VT_CS_SAMPLE_LENGTH; iter++)
    {
        signal[iter].imag = 0;
        signal[iter].real = 0;
    }

    static SPECTOGRAM spectogram_calib_fetch[VT_CS_FFT_LENGTH];
    memset(spectogram_calib_fetch,0,sizeof(SPECTOGRAM)*VT_CS_FFT_LENGTH);

    for (VT_INT iter = 0; iter < calib_ranges; iter++)
    {

        if (cs_repeating_raw_signature_fetch_stored_current_measurement(
                cs_object, adc_read_signal, get_calib_range_freq(iter), VT_CS_SAMPLE_LENGTH))
        {
            continue;
        }
        // [TODO] Add Digital Filter
        for (VT_INT iter1 = 0; iter1 < VT_CS_SAMPLE_LENGTH; iter1++)
        {
            signal[iter1].real = adc_read_signal[iter1];
            signal[iter1].imag = 0;
        }
        calculate_top_N_signal_frequencies(spectogram_calib_fetch, 0, signal, get_calib_range_freq(iter));
        for (VT_INT iter1 = 0; iter1 < VT_CS_MAX_TEST_FREQUENCIES; iter1++)
        {
            for (VT_INT iter2 = 0; iter2 < VT_CS_MAX_TEST_FREQUENCIES; iter2++)
            {
                if (spectogram_calib_fetch[iter1].magnitude > spectogram_calib[iter2].magnitude)
                {
                    spectogram_calib[VT_CS_MAX_TEST_FREQUENCIES - 1].magnitude = spectogram_calib_fetch[iter1].magnitude;
                    spectogram_calib[VT_CS_MAX_TEST_FREQUENCIES - 1].frequency = spectogram_calib_fetch[iter1].frequency;
                    sort_spectogram_amplitude(spectogram_calib, VT_CS_MAX_TEST_FREQUENCIES);
                    break;
                }
            }
        }
    }
    sort_spectogram_amplitude(spectogram_calib, VT_CS_MAX_TEST_FREQUENCIES);

    VTLogDebug("Dominant Signals: \r\n");
    VTLogDebugNoTag("Frequency\t:\tMagnitude \r\n");
#if VT_LOG_LEVEL > 3
    for (VT_INT iter = 0; iter < VT_CS_MAX_TEST_FREQUENCIES; iter++)
    {
        decimal    = spectogram_calib[iter].frequency;
        frac_float = spectogram_calib[iter].frequency - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d : ", decimal, frac);
        decimal    = spectogram_calib[iter].magnitude;
        frac_float = spectogram_calib[iter].magnitude - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d \r\n", decimal, frac);
    }
#endif /* VT_LOG_LEVEL > 2 */


    VT_FLOAT min_ref_freq=VT_CS_ADC_MAX_SAMPLING_FREQ;

    for (VT_UINT iter1 = 0; iter1 < cs_object->raw_signatures_reader->num_repeating_raw_signatures; iter1++){

        if (cs_object->raw_signatures_reader->repeating_raw_signatures[iter1].sampling_frequency < min_ref_freq){
            min_ref_freq=cs_object->raw_signatures_reader->repeating_raw_signatures[iter1].sampling_frequency;
        }
    }

    #if VT_LOG_LEVEL > 3
    VTLogDebugNoTag("\nMIN REF FREQ: \n");

        decimal    = min_ref_freq;
        frac_float = min_ref_freq - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d \n ", decimal, frac);
        
    
    #endif /* VT_LOG_LEVEL > 2 */

    *lowest_sample_freq = VT_CS_ADC_MAX_SAMPLING_FREQ;
    VT_FLOAT new_freq;
    for (VT_INT iter = 0; iter < VT_CS_MAX_TEST_FREQUENCIES; iter++)
    {
        if (spectogram_calib[iter].magnitude == 0)
        {
            break;
        }
        new_freq = get_raw_signature_sample_freq(spectogram_calib[iter].frequency);

        if(new_freq<min_ref_freq){

           
            top_N_sample_frequencies[iter]=0;
        }
        else{
            top_N_sample_frequencies[iter]=new_freq;
        }
        if ((top_N_sample_frequencies[iter] < *lowest_sample_freq) && ((top_N_sample_frequencies[iter] >min_ref_freq)))
        {
            *lowest_sample_freq = top_N_sample_frequencies[iter];
        }
    }

    
    #if VT_LOG_LEVEL > 3
    VTLogDebugNoTag("\nFINAL TOP N FREQ: \n");
    for (VT_INT iter = 0; iter < VT_CS_MAX_TEST_FREQUENCIES; iter++)
    {
        decimal    = top_N_sample_frequencies[iter];
        frac_float = top_N_sample_frequencies[iter] - (VT_FLOAT)decimal;
        frac       = fabsf(frac_float) * 10000;
        VTLogDebugNoTag("%d.%04d , ", decimal, frac);
        
    }
#endif /* VT_LOG_LEVEL > 2 */
}