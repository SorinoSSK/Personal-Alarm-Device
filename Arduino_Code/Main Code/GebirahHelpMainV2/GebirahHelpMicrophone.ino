/**
 * Callback function to process the data from the PDM microphone.
 * NOTE: This callback is executed as part of an ISR.
 * Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void audio_rec_callback(uint16_t *buf, uint32_t buf_len) 
{
    static uint32_t idx = 0;
    // Copy samples from DMA buffer to inference buffer
    if (SendMicRecord)
    {
        if (!Mic.begin()) 
        {
            if (Debug_Status != 0)
            {
                Serial.println("PDM MIC Error!");
            }
        }
        else
        {
            // Convert 12-bit unsigned ADC value to 16-bit PCM (signed) audio value
            // sampleBuffer[idx++] = buf[i];
            sampleBuffer[idx] = filter.step((int16_t)(buf[idx] - 1024) * 16);   //with Filter
            idx += 1;
            if (idx >= mic_config.buf_size) 
            {
                idx = 0;
                //record_ready = true;
                PDMsMicRecs.writeValue(sampleBuffer, sizeof(short)*mic_config.buf_size);
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