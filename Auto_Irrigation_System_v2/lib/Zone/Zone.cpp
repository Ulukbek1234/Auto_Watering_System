#include "Zone.h"

Zone::Zone(int id) : zone_id(id)
{
    // Construct Pump
}

void Zone::saveToEEPROMData(EE_Data_t *eeprom_data)
{
    for(int i = 0; i < nr_pumps; i++)
    {
        eeprom_data->cali_air[i] = soil_sensors[i]->getCaliAir();
        eeprom_data->cali_water[i] = soil_sensors[i]->getCaliWater();
        eeprom_data->total_liters[i] = pumps[i]->getTotalLiter();
        eeprom_data->max_liters[i] = pumps[i]->getMaxLiter();
        eeprom_data->moisture_threshold[i] = MOISTURE_THRESHOLD[i];
        eeprom_data->pump_mode[i] = current_mode[i];
    }
}

void Zone::saveFromEEPROMData(EE_Data_t *eeprom_data)
{
    for(int i = 0; i < nr_pumps; i++)
    {
        // TODO, is this necessary?
        // soil_sensors[i]->setCaliAir(eeprom_data->cali_air[i]); 
        // soil_sensors[i]->setCaliWater(eeprom_data->cali_water[i]); 
        // pumps[i]->setTotalLiters(eeprom_data->total_liters[i]); 
        // pumps[i]->updateMaxLiters(eeprom_data->max_liters[i]); 
        // MOISTURE_THRESHOLD[i] = eeprom_data->moisture_threshold[i]; 
        // current_mode[i] = static_cast<OperationModes> *eeprom_data->pump_mode[i]; 
    }
}


void Zone::resetEEPROM()
{
    // int offset = eeprom_offset_start;
    // eeprom_total_day_progressed = 0.0;

    // EEPROM.put(offset, 0);
    // offset += 4;

    // for(int i = 0; i < nr_pumps; i++)
    // {
    //     // 4 byte offset from total_day_progressed
    //     // increment by float size
    //     eeprom_values[i] = 0.0;
    //     EEPROM.put(offset + (i * 4), 0.0);
    //     offset += 4;
    // }
}

void Zone::addPump(int pin, float max_liters, float total_liters)
{
    pumps[nr_pumps++] = new Pump(pin, max_liters, total_liters);
}

void Zone::addSoilSensor(uint8_t pin, int cali_air, int cali_water)
{
    soil_sensors[nr_soil_sensors++] = new SoilSensor(pin, cali_air, cali_water);
}

void Zone::addWaterLevelSensor(uint8_t pin)
{
    water_level_sensors[nr_water_level_sensors++] = new WaterLevelSensor(pin);
}

void Zone::startAutoIrrigation()
{
    // Check moisture from sensors
    // if below a threshold, must start pumping
    // must not exceed maximum water limit

    updateSensors();    
    for(int i = 0; i < nr_pumps; i++)
    {
        switch (current_mode[i])
        {
            case MODE_OFF:  
                DEBUG_PRINTLN("Pump is off");
                break;
            case MODE_MANUAL:
                DEBUG_PRINTLN("Pump is in MANUAL mode, not auto irrigating.");        
                break;
            case MODE_FLOOD:
                DEBUG_PRINTLN("Pump is in FLOOD mode, irrigating all pumps to max daily limit.");
                pumps[i]->turnOnPump(0.2, false);
                break;
            case MODE_SOIL:
                DEBUG_PRINTLN("Pump is in SOIL mode, irrigating based on soil moisture.");
                DEBUG_PRINT("moisture_percent for sensor: ");
                DEBUG_PRINTLN(moisture_percent[i]);
                if((MOISTURE_THRESHOLD[i] - moisture_percent[i]) > epsilon )
                {
                    pumps[i]->turnOnPump(0.2, false);
                }
                break;
            default:
                break;
        }
    
    }
    
}

void Zone::updateDay()
{
    unsigned long millis_uptime = millis();  // Milliseconds since startup
    day_exact = millis_uptime / 1000.0 / 60.0 / 60.0 / 24.0;  // Convert to days
    DEBUG_PRINT("Updating day -- Days running: ");
    DEBUG_PRINTLN(day_exact);
    total_day_progressed = eeprom_total_day_progressed + day_exact;
    
    if((day_exact - day_progressed) > epsilon)
    {
        DEBUG_PRINTLN("DAY PROGRESSED");
        day_progressed += 1.0;
        for(int i = 0; i < nr_pumps; i++)
        {
            pumps[i]->resetDailyLiter();
        }
    }
}
        
        
String Zone::getData()
{
    String data_names[32];
    String data_values[32];
    int index = 0;


    data_names[index] = "current_day";
    data_values[index++] = String(day_exact, NR_DEC_POINTS);

    data_names[index] = "total_days";
    data_values[index++] = String(total_day_progressed, NR_DEC_POINTS);


    for (int i = 0; i < nr_pumps; i++)
    {
        String pump_pin = String(pumps[i]->getPin());
        data_names[index] = "pump_pin_" + pump_pin;
        data_values[index++] = pump_pin;
        data_names[index] = "current_mode_" + pump_pin;
        data_values[index++] = current_mode[i];
        data_names[index] = "daily_liter_" + pump_pin;
        data_values[index++] = String(pumps[i]->getDailyLiter(), NR_DEC_POINTS);
        data_names[index] = "total_liter_" + pump_pin;
        data_values[index++] = String(pumps[i]->getTotalLiter(), NR_DEC_POINTS);
        data_names[index] = "max_liter_" + pump_pin;
        data_values[index++] = String(pumps[i]->getMaxLiter(), NR_DEC_POINTS);
        
        soil_sensors[i]->checkRawValues();
        String soil_pin = String(soil_sensors[i]->getPin());
        data_names[index] = "moisture_percent_" + soil_pin;
        data_values[index++] = soil_sensors[i]->getMoisturePercent();
        data_names[index] = "moisture_raw_" + soil_pin;
        data_values[index++] = soil_sensors[i]->getMoistureRaw(); 
        data_names[index] = "moisture_threshold_" + soil_pin;
        data_values[index++] = MOISTURE_THRESHOLD[i];
    }

    return Utils::parseDataForWriting(data_names, data_values, index);
}


void Zone::setOperationMode(OperationModes mode, int pump_id)
{
    for(int i = 0; i < nr_pumps; i++)
    {
        if(pumps[i]->getPin() == pump_id)
        {
            current_mode[i] = mode;
            switch (current_mode[i])
            {
                case MODE_OFF:
                    pumps[i]->deactivatePump();
                    break;
                case MODE_MANUAL:
                case MODE_FLOOD:
                case MODE_SOIL:
                    pumps[i]->activatePump();
                    break;
                default:
                    break;
            }
        }
    }
        
}

void Zone::updateSensors()
{
    for(int i = 0; i < nr_soil_sensors; i++)
    {
        DEBUG_PRINT("soil_sensor id (Analog): ");
        DEBUG_PRINTLN(soil_sensors[i]->getPin());

        soil_sensors[i]->checkRawValues();
        moisture_percent[i] = soil_sensors[i]->getMoisturePercent();
    }

    for(int i = 0; i < nr_water_level_sensors; i++)
    {
        DEBUG_PRINT("water_level_sensor id (Analog): ");
        DEBUG_PRINTLN(water_level_sensors[i]->getPin());

        water_level_sensors[i]->checkRawValues();
        water_level_percent[i] = water_level_sensors[i]->getMoisturePercent();
        if(water_level_percent[i] > 90)
        {
            DEBUG_PRINTLN("Water level too high, skipping irrigation.");
            pumps[i]->deactivatePump();
        }
        else 
        {
            pumps[i]->activatePump();
        }
    }
}

void Zone::manualIrrigation(int pump_id, float amount)
{
    for(int i = 0; i < nr_pumps; i++)
    {
        if(pumps[i]->getPin() == pump_id){
            pumps[i]->turnOnPump(amount, true);
            return;
        }
    }
}



void Zone::changeDailyLimit(int pump_id, float new_limit)
{
    for(int i = 0; i < nr_pumps; i++)
    {
        if(pumps[i]->getPin() == pump_id){
            pumps[i]->updateMaxLiters(new_limit);
            return;
        }
    }
}

void Zone::changeMoistureThreshold(float new_limit, int pump_id)
{
    for(int i = 0; i < nr_pumps; i++)
    {
        if(pumps[i]->getPin() == pump_id)
        {
            MOISTURE_THRESHOLD[i] = new_limit;

        }
    }
}

bool Zone::caliSoilInAir(int soil_pin)
{
    for(int i = 0; i < nr_soil_sensors; i++)
    {
        if(soil_sensors[i]->getPin() == soil_pin)
        {
            soil_sensors[i]->calibrateInAir();
            return true;
        }
    }
    return false;
}

bool Zone::caliSoilInWater(int soil_pin)
{
    for(int i = 0; i < nr_soil_sensors; i++)
    {
        if(soil_sensors[i]->getPin() == soil_pin)
        {
            soil_sensors[i]->calibrateInWater();
            return true;
        }
    }
    return false;
}