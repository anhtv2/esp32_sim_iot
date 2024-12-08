#pragma once

namespace SerialCommand{
  void (*serial_function) ;
  void test(){
    if(serial_function!=null){
      serial_function();
    }
  }
}