#include "Zone.h"

Zone::Zone(int id) : zone_id(id)
{
    // Construct Pump
    EEPROM.get(0, old_total_day_progressed);

    EEPROM.get(4, eeprom_values[0]);
    EEPROM.get(8, eeprom_values[1]);
    EEPROM.get(12, eeprom_values[2]);
    EEPROM.get(16, eeprom_values[3]);
}

void Zone::addPump(int pin, float max_liters)
{
    pumps[nr_pumps++] = new Pump(pin, max_liters);
}

void Zone::addSoilSensor(uint8_t pin)
{
    soil_sensors[nr_soil_sensors++] = new SoilSensor(pin);
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
            pumps[i]->turnOnPump(0.1);
        }
        break;
    case MODE_SOIL:
        DEBUG_PRINTLN("Zone is in SOIL mode, irrigating based on soil moisture.");
        for(int i = 0; i < nr_pumps; i++)
        {
            DEBUG_PRINT("moisture_percent for sensor: ");
            DEBUG_PRINTLN(moisture_percent[i]);
            if(moisture_percent[i] < MOISTURE_THRESHOLD)
            {
                pumps[i]->turnOnPump(0.1);
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
    total_day_progressed = old_total_day_progressed + day_exact;
    
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
        // data_names[index] = "is_active_" + pump_pin;
        // data_values[index++] = String(pumps[i]->getIsActive());
        data_names[index] = "eeprom_val_" + pump_pin;
        data_values[index++] = eeprom_values[i];
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

void Zone::saveToEEPROM()
{
    // EEPROM addresses
    // sizeof(float) == 4 bytes
    // 0-3 total_day_progressed
    // 4-7 pump_8_total
    // 8-11 pump_9_total
    // 12-15 pump_10_total
    // 16-19 pump_11_total

    updateDay();
    EEPROM.put(0, total_day_progressed);

    for(int i = 0; i < nr_pumps; i++)
    {
        // 4 byte offset from total_day_progressed
        // increment by float size
        float total_value = eeprom_values[i] + pumps[i]->getTotalLiter();
        EEPROM.put(4 + (i * 4), total_value);
    }
}