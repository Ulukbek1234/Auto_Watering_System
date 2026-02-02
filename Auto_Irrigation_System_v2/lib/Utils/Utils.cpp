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

bool Utils::findDataFromMessage(
    const String& input,
    const String& key,
    String& output
) {
    int keyIndex = input.indexOf(key);
    if (keyIndex == -1) return false;

    int start = keyIndex + key.length();
    int end = input.indexOf(',', start);

    output = (end == -1)
        ? input.substring(start)
        : input.substring(start, end);

    output.trim();
    return true;
}