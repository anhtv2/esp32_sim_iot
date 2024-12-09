#pragma once

#include <string.h>
#include <Arduino.h>
#include <vector>

namespace SerialCommand
{
  std::vector<String> read_and_split()
  {
    std::vector<String> words;

    if (Serial.available())
    {
      String line = Serial.readStringUntil('\n');
      line.trim(); // Remove leading and trailing whitespace

      int spaceIndex;
      while ((spaceIndex = line.indexOf(' ')) != -1)
      {
        String word = line.substring(0, spaceIndex);
        words.push_back(word); // Add the word to the vector

        // Remove the word and the space from the line
        line = line.substring(spaceIndex + 1);
      }

      // Add the last word (or the only word if there were no spaces)
      words.push_back(line);
    }

    return words;
  }
}