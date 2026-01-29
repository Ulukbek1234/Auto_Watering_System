#include "Utils.h"


float Utils::floatMap(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

String Utils::parseDataForWriting(String data_names[], String data_values[], int size)
{
    String output;
    for(int i = 0; i < size; i++)
    {
        output += data_names[i] + ": " + data_values[i] + ", ";
    }
    return output;
}

String Utils::parseDataForWriting(String data_name, String data_value)
{
    return data_name + ": " + data_value + ", ";
}

String Utils::parseDataForWriting(Dictionary data)
{
    return data.key_ + ": " + data.value_ + ", ";
}


String Utils::addCSVHeader(const char *header[], int size)
{
    String output = "";
    for(int i = 0; i < size; i++)
    {
        output += header[i];
        if(i < size - 1)
            output += ", ";
    }
    return output;
}

String Utils::findDataFromMessage(String input, String data) {
    String output;
    int data_index = input.indexOf(data);
    if (data_index != -1) {
        int start_index = data_index + data.length();
        int comma_index = input.indexOf(',', start_index);
        if (comma_index != -1) {
            output = input.substring(start_index, comma_index);
        } else {
            // If no comma is found, extract until the end of the string
            output = input.substring(start_index);
        }
    }
    return output;
}