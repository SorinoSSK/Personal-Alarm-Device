/**
 * Callback function to process the data from the PDM microphone.
 * NOTE: This callback is executed as part of an ISR.
 * Therefore using `Serial` to print messages inside this function isn't supported.
 * */
static void audio_rec_callback(uint16_t *buf, uint32_t buf_len) 
{    
    // Copy samples from DMA buffer to inference buffer

    static uint32_t idx = 0;

    if (recording) {
        for (uint32_t i = 0; i < buf_len; i++) 
        {
        
        #if defined(WIO_TERMINAL)
            // Convert 12-bit unsigned ADC value to 16-bit PCM (signed) audio value
            recording_buf[idx++] = filter.step((int16_t)(buf[i] - 1024) * 16);    //with Filter
            //recording_buf[idx++] = (int16_t)(buf[i] - 1024) * 16;               // without filter
        #elif defined(ARDUINO_ARCH_NRF52840)
            recording_buf[idx++] = buf[i];
        #endif
                
            if (idx >= SAMPLES)
            { 
                idx = 0;
                recording = 0;
                record_ready = true;
                break;
            } 
        }
    }
}

// void microphoneFunction()
// {
//     if (samplesRead) 
//     {
//         // Use memcpy for bulk copy
//         memcpy(sampleBuffer1 + sample_cnt, sampleBuffer, samplesRead * sizeof(short));

//         sample_cnt += samplesRead;

//         if (sample_cnt >= SAMPLES) 
//         {
//             // Consider renaming MicRecordRdy for clarity
//             MicRecordRdy = true;
//             sample_cnt = 0;
//         }
//         samplesRead = 0;
//     }
// }