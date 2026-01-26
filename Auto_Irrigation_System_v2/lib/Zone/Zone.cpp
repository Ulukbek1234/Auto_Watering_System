#include "Zone.h"

Zone::Zone() 
{
    // Construct Pump

}

void Zone::addPump(int pin, float max_liters)
{
    pumps[nr_pumps++] = new Pump(pin, max_liters);
}

void Zone::addSoilSensor(uint8_t pin)
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


    float moisture_percent[nr_soil_sensors] = {0.0f};
    for(size_t i = 0; i < nr_soil_sensors; i++)
    {
        DEBUG_PRINT("soil_sensor id (Analog): ");
        DEBUG_PRINTLN(soil_sensors[i]->getPin());

        soil_sensors[i]->checkRawValues();
        moisture_percent[i] = soil_sensors[i]->getMoisterPercent();
    }

    for(size_t i = 0; i < nr_pumps; i++)
    {
        DEBUG_PRINT("moisture_percent for sensor: ");
        DEBUG_PRINTLN(moisture_percent[i]);
        if(moisture_percent[i] < MOISTURE_THRESHOLD)
        {
            pumps[i]->activatePump(0.1);
        }
    }
}

void Zone::updateDay()
{
    unsigned long millis_uptime = millis();  // Milliseconds since startup
    day_exact = millis_uptime / 1000.0 / 60.0 / 60.0 / 24.0;  // Convert to days
    DEBUG_PRINT("Updating day -- Days running: ");
    DEBUG_PRINTLNN(day_exact, 10);
    
    if((day_exact - day_progressed) > epsilon)
    {
        DEBUG_PRINTLN("DAY PROGRESSED");
        day_progressed += 1.0;
        for(size_t i = 0; i < nr_pumps; i++)
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


    for (size_t i = 0; i < nr_soil_sensors; i++)
    {
        data_names[index] = "soil_sensor_pin";
        data_values[index++] = soil_sensors[i]->getPin();
        data_names[index] = "moister_percent";
        data_values[index++] = soil_sensors[i]->getMoisterPercent();
    }

    for (size_t i = 0; i < nr_pumps; i++)
    {
        data_names[index] = "pump_pin";
        data_values[index++] = pumps[i]->getPin();
        data_names[index] = "daily_liter";
        data_values[index++] = String(pumps[i]->getDailyLiter(), NR_DEC_POINTS);
        data_names[index] = "total_liter";
        data_values[index++] = String(pumps[i]->getTotalLiter(), NR_DEC_POINTS);
        data_names[index] = "max_liter";
        data_values[index++] = String(pumps[i]->getMaxLiter(), NR_DEC_POINTS);
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

void Zone::resetDayProgression()
{
    day_progressed = 0.0;
    day_exact = 0.0;
}