static void addBackslashes(String &originalString) {
  // Create a temporary string to store the modified string
  String modifiedString = "";

  // Iterate through each character in the original string
  for (int i = 0; i < originalString.length(); i++) {
    // Check if the current character is a double quote
    if (originalString[i] == '"') {
      // If yes, add a backslash before the double quote
      modifiedString += '\\';
    }

    // Add the current character to the modified string
    modifiedString += originalString[i];
  }

  // Update the original string with the modified string
  originalString = modifiedString;
}

static void stringToUint16(String &originalString, uint16_t *buff) {
    for (int i = 0; i < originalString.length(); i++) {
        buff[i] = static_cast<uint16_t>(originalString[i]);
    }
}

static String uint16ToString(uint16_t *buff) {
    String jsonString;
    for (size_t i = 0; i < MemToUse; i++) {
        if (pBuf[i] == 65535)
        {
            break;
        } else { 
            jsonString += char(pBuf[i]); 
            }
    }
    return jsonString;
}

static void qspi_handler(nrfx_qspi_evt_t event, void *p_context) {
  // UNUSED_PARAMETER(p_context);
  // Serial.println("QSPI Interrupt");
  // if (event == NRFX_QSPI_EVENT_DONE) {
  //   QSPI_HasFinished = true;
  // }
}

static void QSPI_Status(char ASender[]) {  // Prints the QSPI Status
    if (Debug_Status == 2)
    {
        Serial.print("(");
        Serial.print(ASender);
        Serial.print(") QSPI is busy/idle ... Result = ");
        Serial.println(nrfx_qspi_mem_busy_check() & 8);
        Serial.print("(");
        Serial.print(ASender);
        Serial.print(") QSPI Status flag = 0x");
        Serial.print(NRF_QSPI->STATUS, HEX);
        Serial.print(" (from NRF_QSPI) or 0x");
        Serial.print(*QSPI_Status_Ptr, HEX);
        Serial.println(" (from *QSPI_Status_Ptr)");
    }
}

static void QSPI_PrintData(uint16_t *AnAddress, uint32_t AnAmount) {
    uint32_t i;

    Serial.print("Data :");
    for (i = 0; i < AnAmount; i++) {
        Serial.print(" 0x");
        Serial.print(*(AnAddress + i), HEX);
    }
    Serial.println("");
}

static void QSPI_PrintDataChar(uint16_t *AnAddress) {
    uint32_t i;

    Serial.print("Data :");
    for (i = 0; i < MemToUse/2; i++) {
        Serial.print((char)*(AnAddress + i));
    }
    Serial.println("");
}

static nrfx_err_t QSPI_IsReady() {
    if (((*QSPI_Status_Ptr & 8) == 8) && (*QSPI_Status_Ptr & 0x01000000) == 0) {
        return NRFX_SUCCESS;
    } else {
        return NRFX_ERROR_BUSY;
    }
}

static nrfx_err_t QSPI_WaitForReady() {
    while (QSPI_IsReady() == NRFX_ERROR_BUSY) {
        if (Debug_Status == 2) {
            Serial.print("*QSPI_Status_Ptr & 8 = ");
            Serial.print(*QSPI_Status_Ptr & 8);
            Serial.print(", *QSPI_Status_Ptr & 0x01000000 = 0x");
            Serial.println(*QSPI_Status_Ptr & 0x01000000, HEX);
        }
        QSPI_Status("QSPI_WaitForReady");
    }
    return NRFX_SUCCESS;
}

static nrfx_err_t QSPI_Initialise() {  // Initialises the QSPI and NRF LOG
    uint32_t Error_Code;

    NRF_LOG_INIT(NULL);  // Initialise the NRF Log
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    // QSPI Config
    QSPIConfig.xip_offset = NRFX_QSPI_CONFIG_XIP_OFFSET;
    QSPIConfig.pins = {
        // Setup for the SEEED XIAO BLE - nRF52840
        .sck_pin = 21,
        .csn_pin = 25,
        .io0_pin = 20,
        .io1_pin = 24,
        .io2_pin = 22,
        .io3_pin = 23,
    };
    QSPIConfig.irq_priority = (uint8_t)NRFX_QSPI_CONFIG_IRQ_PRIORITY;
    QSPIConfig.prot_if = {
        // .readoc     = (nrf_qspi_readoc_t)NRFX_QSPI_CONFIG_READOC,
        .readoc = (nrf_qspi_readoc_t)NRF_QSPI_READOC_READ4O,
        // .writeoc    = (nrf_qspi_writeoc_t)NRFX_QSPI_CONFIG_WRITEOC,
        .writeoc = (nrf_qspi_writeoc_t)NRF_QSPI_WRITEOC_PP4O,
        .addrmode = (nrf_qspi_addrmode_t)NRFX_QSPI_CONFIG_ADDRMODE,
        .dpmconfig = false,
    };
    QSPIConfig.phy_if.sck_freq = (nrf_qspi_frequency_t)NRF_QSPI_FREQ_32MDIV1;  // I had to do it this way as it complained about nrf_qspi_phy_conf_t not being visible
    // QSPIConfig.phy_if.sck_freq   = (nrf_qspi_frequency_t)NRFX_QSPI_CONFIG_FREQUENCY;
    QSPIConfig.phy_if.spi_mode = (nrf_qspi_spi_mode_t)NRFX_QSPI_CONFIG_MODE;
    QSPIConfig.phy_if.dpmen = false;
    // QSPI Config Complete
    // Setup QSPI to allow for DPM but with it turned off
    QSPIConfig.prot_if.dpmconfig = true;
    NRF_QSPI->DPMDUR = (QSPI_DPM_ENTER << 16) | QSPI_DPM_EXIT;  // Found this on the Nordic Q&A pages, Sets the Deep power-down mode timer
    Error_Code = 1;
    while (Error_Code != 0) {
        Error_Code = nrfx_qspi_init(&QSPIConfig, NULL, NULL);
        if (Error_Code != NRFX_SUCCESS) {
            if (Debug_Status == 2) {
                Serial.print("(QSPI_Initialise) nrfx_qspi_init returned : ");
                Serial.println(Error_Code);
            }
        } else {
            if (Debug_Status == 2) {
                Serial.println("(QSPI_Initialise) nrfx_qspi_init successful");
            }
        }
    }
    QSPI_Status("QSPI_Initialise (Before QSIP_Configure_Memory)");
    QSIP_Configure_Memory();
    if (Debug_Status == 2) {
        Serial.println("(QSPI_Initialise) Wait for QSPI to be ready ...");
    }
    NRF_QSPI->TASKS_ACTIVATE = 1;
    QSPI_WaitForReady();
    if (Debug_Status == 2) {
        Serial.println("(QSPI_Initialise) QSPI is ready");
    }
    return QSPI_IsReady();
}

static void QSPI_Erase(uint32_t AStartAddress) {
    uint32_t TimeTaken;
    bool QSPIReady = false;
    bool AlreadyPrinted = false;

    if (Debug_Status == 2) {
        Serial.println("(QSPI_Erase) Erasing memory");
    }
    while (!QSPIReady) {
        if (QSPI_IsReady() != NRFX_SUCCESS) {
            if (!AlreadyPrinted) {
                QSPI_Status("QSPI_Erase (Waiting)");
                AlreadyPrinted = true;
            }
        } else {
            QSPIReady = true;
            QSPI_Status("QSPI_Erase (Waiting Loop Breakout)");
        }
    }
    QSPI_Status("QSPI_Erase (Finished Waiting)");
    if (Debug_Status == 2) {
        TimeTaken = millis();
    }
    if (nrfx_qspi_erase(NRF_QSPI_ERASE_LEN_64KB, AStartAddress) != NRFX_SUCCESS) {
        if (Debug_Status == 2) {
            Serial.print("(QSPI_Initialise_Page) QSPI Address 0x");
            Serial.print(AStartAddress, HEX);
            Serial.println(" failed to erase!");
        }
    } else {
        if (Debug_Status == 2) {
            TimeTaken = millis() - TimeTaken;
            Serial.print("(QSPI_Initialise_Page) QSPI took ");
            Serial.print(TimeTaken);
            Serial.println("ms to erase a 64Kb page");
        }
    }
}

static void QSIP_Configure_Memory() {
    // uint8_t  temporary = 0x40;
    uint8_t temporary[] = { 0x00, 0x02 };
    uint32_t Error_Code;

    QSPICinstr_cfg = {
        .opcode = QSPI_STD_CMD_RSTEN,
        .length = NRF_QSPI_CINSTR_LEN_1B,
        .io2_level = true,
        .io3_level = true,
        .wipwait = QSPIWait,
        .wren = true
    };
    QSPI_WaitForReady();
    if (nrfx_qspi_cinstr_xfer(&QSPICinstr_cfg, NULL, NULL) != NRFX_SUCCESS) {  // Send reset enable
        if (Debug_Status == 2) {
        Serial.println("(QSIP_Configure_Memory) QSPI 'Send reset enable' failed!");
        }
    } else {
        QSPICinstr_cfg.opcode = QSPI_STD_CMD_RST;
        QSPI_WaitForReady();
        if (nrfx_qspi_cinstr_xfer(&QSPICinstr_cfg, NULL, NULL) != NRFX_SUCCESS) {  // Send reset command
        if (Debug_Status == 2) {
            Serial.println("(QSIP_Configure_Memory) QSPI Reset failed!");
        }
        } else {
            QSPICinstr_cfg.opcode = QSPI_STD_CMD_WRSR;
            QSPICinstr_cfg.length = NRF_QSPI_CINSTR_LEN_3B;
            QSPI_WaitForReady();
            if (nrfx_qspi_cinstr_xfer(&QSPICinstr_cfg, &temporary, NULL) != NRFX_SUCCESS) {  // Switch to qspi mode
                if (Debug_Status == 2) {
                    Serial.println("(QSIP_Configure_Memory) QSPI failed to switch to QSPI mode!");
                }
            } else {
                QSPI_Status("QSIP_Configure_Memory");
            }
        }
    }
}

// Write json data to memory
static void storeJSONToMemory()
{
    String jsonString;
    serializeJson(jsonData, jsonString);
    // Print String only it debugging
    if (Debug_Status == 2)
    {
        Serial.println(jsonString);
    }
    // Wait Memory to be ready for erasing and then writing.
    QSPI_WaitForReady();
    QSPI_Erase(0);
    QSPI_WaitForReady();
    // Convert JSON string back to unsigned int 16 for writing
    stringToUint16(jsonString, pBuf);
    if (Debug_Status == 2)
    {
        QSPI_PrintDataChar(&pBuf[0]);
    }
    // Write JSON into memory
    Error_Code = nrfx_qspi_write(pBuf, MemToUse, 0x0);
    QSPI_WaitForReady();
    // If no error, indicate successful write. Else indicate fail write and proceed.
    if (Error_Code == 0)
    {
        if (Debug_Status != 0)
        {
            Serial.println("Data written successfully.");
        }
    }
    else
    {
        if (Debug_Status != 0)
        {
            Serial.println("Data write failed.");
        }
    }
}

static void resetMemory()
{
    jsonData["DeviceToken"] = "6814a608-5385-4ce1-917d-c51840bb75a0";
    jsonData["DResetToken"] = "52702af6-87d2-4290-a01b-036c88ec2811";
    jsonData["UserDCToken"] = "";
    jsonData["NoOfFallDet"] = "0";
    jsonData["FastChargng"] = "1";
    jsonData["BattVoltage"] = "0";
    jsonData["BattBounded"] = "1";
    jsonData["FallDetects"] = "1";
    DynamicJsonDocument doc(NoOfEmergencyContact);
    JsonArray EmergencyNoArr = doc.createNestedArray("EmergencyNoArr");
    jsonData["EmergencyNo"] = EmergencyNoArr;
    storeJSONToMemory();
    resetDevice = false;
}

static void QSPIInit()
{
    if (QSPI_Initialise() != NRFX_SUCCESS)
    {
        if (Debug_Status != 0)
        {
            Serial.println("QSPI Memory failed to start!");
        }
      }
    else
    {
        if (Debug_Status != 0)
        {
            Serial.println("QSPI initialised and ready!");
        }
    }
}

static void QSPIMemoryCheck()
{
    // Wait for Memory to be ready for reading
    QSPI_WaitForReady();
    Error_Code = nrfx_qspi_read(pBuf, MemToUse, 0x0);
    // Wait for Memory to be ready after reading
    QSPI_WaitForReady();
    // If Error Code is 0 do work, else throw an error and proceed
    if (Error_Code == 0)
    {
        if (Debug_Status != 0)
        {
            Serial.println("Initialising Data...");
        }
        // DeserializationError error = deserializeJson(jsonData, (char *)&pBuf);
        DeserializationError error = deserializeJson(jsonData, uint16ToString(pBuf));
        // Check if data is in JSON format. If not then attempt to override with one.
        if (error)
        {
            // Print Error
            if (Debug_Status != 0)
            {
                Serial.print(F("JSON deserialisation failed: "));
                Serial.println(error.f_str());
            }
            // If Debugging mode, turn on full print.
            if (Debug_Status == 2)
            {
                Serial.println(F("Reading stored data..."));
                QSPI_PrintDataChar(&pBuf[0]);
            }
            // Initialise JSON and write it into JSON string
            if (Debug_Status != 0)
            {
                Serial.println(F("Replacing JSON and writing into memory..."));
            }
            resetMemory();
        }
        else
        {
            // If successful, print ok.
            if (Debug_Status != 0)
            {
                Serial.println("Data in memory is normal!");
            }
            // If debug, print JSON string
            if (Debug_Status == 2)
            {
                String jsonString;
                serializeJson(jsonData, jsonString);
                Serial.println(jsonString);
            }
        }
    }
    else
    {
        if (Debug_Status != 0)
        {
            Serial.println("Error reading from Flash Memory! Skipping memory...");
        }
    }
}