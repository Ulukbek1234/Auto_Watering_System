#include "Zone.h"

Zone::Zone(int id) : zone_id(id)
{
    // Construct Pump

    // EEPROM.get(absolute_offset, eeprom_total_day_progressed);
    // absolute_offset += 4;

    // for(int i = 0; i < 4; i++)
    // {
    //     EEPROM.get(absolute_offset, eeprom_values[i]);
    //     absolute_offset += 4;
    // }
    // eeprom_offset_end = absolute_offset;
}

void Zone::addPump(int pin, float max_liters)
{
    pumps[nr_pumps++] = new Pump(pin, max_liters);
    // EEPROM.getAddress();
}

void Zone::addSoilSensor(uint8_t pin, int cali_air, int cali_water)
{
    soil_sensors[nr_soil_sensors++] = new SoilSensor(pin, cali_air, cali_water);
}

void Zone::addWaterLevelSensor(uint8_t pin)
{
    water_level_sensors[nr_water_level_sensors++] = new WaterLevelSensor(pin);
}

void Zone::startAutoIrrigating()
{
    // Check moisture from sensors
    // if below a threshold, must start pumping
    // must not exceed maximum water limit

    // TODO change nr_pumps for better solution
    // ISSUE must connect sensors to pumps or vice versa
    // multiple pumps can be attached to one sensor

    if (current_mode == MODE_OFF) {
        DEBUG_PRINTLN("Zone is OFF, not auto irrigating.");
        return;
    }

    updateSensors();    

    switch (current_mode)
    {
    case MODE_MANUAL:
        DEBUG_PRINTLN("Zone is in MANUAL mode, not auto irrigating.");        
        break;
    case MODE_FLOOD:
        DEBUG_PRINTLN("Zone is in FLOOD mode, irrigating all pumps to max daily limit.");
        for(int i = 0; i < nr_pumps; i++)
        {
            pumps[i]->turnOnPump(0.2, false);
        }
        break;
    case MODE_SOIL:
        DEBUG_PRINTLN("Zone is in SOIL mode, irrigating based on soil moisture.");
        for(int i = 0; i < nr_pumps; i++)
        {
            DEBUG_PRINT("moisture_percent for sensor: ");
            DEBUG_PRINTLN(moisture_percent[i]);
            if((MOISTURE_THRESHOLD - moisture_percent[i]) > epsilon )
            {
                pumps[i]->turnOnPump(0.2, false);
            }
        }
        break;
    default:
        break;
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

    data_names[index] = "current_mode_" + String(zone_id);
    data_values[index++] = current_mode;

    for (int i = 0; i < nr_soil_sensors; i++)
    {
        soil_sensors[i]->checkRawValues();
        String soil_pin = String(soil_sensors[i]->getPin());
        data_names[index] = "moisture_percent_" + soil_pin;
        data_values[index++] = soil_sensors[i]->getMoisturePercent();
        data_names[index] = "moisture_raw_" + soil_pin;
        data_values[index++] = soil_sensors[i]->getMoistureRaw(); 
    }

    for (int i = 0; i < nr_pumps; i++)
    {
        String pump_pin = String(pumps[i]->getPin());
        data_names[index] = "pump_pin_" + pump_pin;
        data_values[index++] = pump_pin;
        data_names[index] = "daily_liter_" + pump_pin;
        data_values[index++] = String(pumps[i]->getDailyLiter(), NR_DEC_POINTS);
        data_names[index] = "total_liter_" + pump_pin;
        data_values[index++] = String(pumps[i]->getTotalLiter(), NR_DEC_POINTS);
        data_names[index] = "max_liter_" + pump_pin;
        data_values[index++] = String(pumps[i]->getMaxLiter(), NR_DEC_POINTS);
    }

    for (int i = 0; i < nr_water_level_sensors; i++)
    {
        String water_pin = String(water_level_sensors[i]->getPin());
        data_names[index] = "water_level_sensor_pin_" + water_pin;
        data_values[index++] = water_level_sensors[i]->getPin();
        data_names[index] = "moisture_percent_" + water_pin;
        data_values[index++] = water_level_sensors[i]->getMoisturePercent();
        data_names[index] = "moisture_raw_" + water_pin;
        data_values[index++] = water_level_sensors[i]->getMoistureRaw();
    }

    return Utils::parseDataForWriting(data_names, data_values, index);
}


void Zone::setOperationMode(OperationModes mode)
{
    current_mode = mode;
    switch (current_mode)
    {
    case MODE_OFF:
        for(int i = 0; i < nr_pumps; i++)
        {
            pumps[i]->deactivatePump();
        }
        break;
    case MODE_MANUAL:
    case MODE_FLOOD:
    case MODE_SOIL:
        for(int i = 0; i < nr_pumps; i++)
        {
            pumps[i]->activatePump();
        }
        break;
    default:
        break;
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

void Zone::saveToEEPROM(EE_Data_t *eeprom_data)
{
    for(int i = 0; i < nr_soil_sensors; i++)
    {
        eeprom_data->cali_air[i] = soil_sensors[i]->getCaliAir();
        eeprom_data->cali_water[i] = soil_sensors[i]->getCaliWater();
    }
    for(int i = 0; i < nr_pumps; i++)
    {
        // TODO gotta fix the += makes wrong value if not reset
        // eeprom_data->total_liters[i] += pumps[i]->getTotalLiter();
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