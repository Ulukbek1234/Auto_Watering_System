#include "Zone.h"

Zone::Zone() 
{
    // Construct Pump
    Serial.println("Help");
}

void Zone::addPump(int pin, float max_liters)
{
    pumps[nr_pumps++] = new Pump(pin, max_liters);
}

void Zone::addSoilSensor(int pin)
{
    soil_sensors[nr_soil_sensors++] = new SoilSensor(pin);
}

void Zone::startAutoIrrigating()
{
    // Check moisture from sensors
    // if below a threshold, must start pumping
    // must not exceed maximum water limit

    // TODO change nr_pumps for better solution
    // ISSUE must connect sensors to pumps or vice versa
    // multiple pumps can be attached to one sensor
    for(size_t i = 0; i < ARRAY_SIZE; i++)
    {
        soil_sensors[i]->checkRawValues();
        float moister_threshold = 0.1;        

        // if(soil_sensors[i]->getMoisterPercent() < moister_threshold)
        {
            // how long should the pumps activate?
            pumps[i]->activatePump(0.2);
            pumps[i]->deactivatePump();
        }
    }
}

void Zone::updateDay()
{
    unsigned long millis_uptime = millis();  // Milliseconds since startup
    day_exact = millis_uptime / 1000.0 / 60 / 60 / 24;  // Convert to days
    DEBUG_PRINT("Updating day -- Days running: ");
    DEBUG_PRINTLN(day_exact);

    if((day_exact - day) > 1.0)
    {
        DEBUG_PRINTLN("DAY PROGRESSED");
        day += 1.0;
        for(size_t i = 0; i < ARRAY_SIZE; i++)
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


    for (size_t i = 0; i < ARRAY_SIZE; i++)
    {
        data_names[index] = "pump_pin";
        data_values[index++] = pumps[i]->getPin();
        data_names[index] = "daily_liter";
        data_values[index++] = String(pumps[i]->getDailyLiter(), NR_DEC_POINTS);


        data_names[index] = "soil_sensor_pin";
        data_values[index++] = soil_sensors[i]->getPin();
        data_names[index] = "moister_percent";
        data_values[index++] = soil_sensors[i]->getMoisterPercent();
    }
    return parseDataForWriting(data_names, data_values, index);
}

String Zone::parseDataForWriting(String data_names[], String data_values[], int size)
{
    String output;
    for(int i = 0; i < size; i++)
    {
        output += data_names[i] + ": " + data_values[i] + ", ";
    }
    return output;
}

