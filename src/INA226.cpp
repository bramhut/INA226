#include "INA226.h"

INA226::INA226(uint8_t SDA, uint8_t SCL, float MAX_CURRENT, float SHUNT_RES, uint8_t ADDR)
{
	_SDA = SDA;
	_SCL = SCL;
	_SHUNT_RES = SHUNT_RES;
	_ADDR = ADDR;
	if (_SHUNT_RES < (0.00512 / (1 << 14) ) ){
		#ifdef INA226_ENABLE_DEBUG
		Serial.println("[INA226] Shunt resistance too low, incorrect readings will be made");
		#endif
		_SHUNT_RES = 0.00512 / (1 << 14);
	}
	_MAX_CURRENT = pow(2,ceil(log2(MAX_CURRENT)));	// Round the max current up to the nearest power of 2
	#ifdef INA226_ENABLE_DEBUG
	Serial.println("[INA226] Max current set to "); Serial.print(_MAX_CURRENT); Serial.println(" A");
	#endif
}


bool INA226::begin(){
	Wire.begin(_SDA, _SCL);
	Wire.beginTransmission(_ADDR);
	int8_t returnCode;
	if ((returnCode = Wire.endTransmission()) == 0){	// Connection succesful
		getConfig();
		setMaxCurrent();

		return true;
	} else {
		#ifdef INA226_ENABLE_DEBUG
		Serial.print("[INA226] Wire begin failed, return code "); Serial.println(returnCode);
		#endif
		return false;
	}
	
}

// PUBLIC FUNCTIONS

// Get the power (W)
float INA226::getPower(){
	setRegister(register_t::POWER);
	return getInt() * 25.0 * currentLSB();
}

// Get the bus voltage (V)
float INA226::getBusVoltage(){
	setRegister(register_t::BUS_V);
	return getInt() * 0.00125F;	// 1.25 mV LSB
}

// Get the current (A)
float INA226::getCurrent(){
	setRegister(register_t::CURRENT);
	return getInt() * currentLSB();
}

// Get the shunt voltage (V)
float INA226::getShuntVoltage(){
	setRegister(register_t::SHUNT_V);
	return getInt() * 0.0000025F;	// 2.5 uV LSB
}

// Set the bus conversion time
void INA226::setBusConversionTime(convTime_t time){
	config.busConversionTime = static_cast<uint8_t>(time);
	setConfig();
}

// Set the shunt conversion time
void INA226::setShuntConversionTime(convTime_t time){
	config.shuntConversionTime = static_cast<uint8_t>(time);
	setConfig();
}

// Set the averaging
void INA226::setAveraging(avg_t avg){
	config.averaging = static_cast<uint8_t>(avg);
	setConfig();
}

// PRIVATE FUNCTIONS

// Set config
void INA226::setConfig(){
	setRegister(register_t::CONFIGURATION);
	writeUInt(config.serialize());
	
}

void INA226::getConfig(){
	setRegister(register_t::CONFIGURATION);
	config.deserialize(getUInt());
}

void INA226::setMaxCurrent(){
	float CAL = 0.00512 / (currentLSB() * _SHUNT_RES);
	if (CAL > (1 << 15)){
		_MAX_CURRENT *= 2;	// Double maximum current if needed
		setMaxCurrent();
		#ifdef INA226_ENABLE_DEBUG
		Serial.print("[INA226] Maximum current too low for accurate readings, maximum current has been doubled");
		#endif
		return;
	}
	setRegister(register_t::CALIBRATION);
	#ifdef INA226_ENABLE_DEBUG
	Serial.print("[INA226] Setting calibration register to "); Serial.print((int) round(CAL)); Serial.print(" (maximum current "); Serial.print(_MAX_CURRENT,4); Serial.println(" A)");
	#endif
	writeUInt(round(CAL));
}

// Set the register pointer on the INA226 to the given register
void INA226::setRegister(const register_t reg){
	if (regPointer != reg){
		Wire.beginTransmission(_ADDR);
		Wire.write(static_cast<uint8_t>(reg));
		Wire.endTransmission();
		regPointer = reg;
	}
}

int16_t INA226::getInt(){
	Wire.requestFrom(_ADDR, (uint8_t) 2);
	uint8_t MSB = Wire.read();
	uint8_t LSB = Wire.read();
	return (int16_t) MSB << 8 | (int16_t) LSB;
}

void INA226::writeUInt(uint16_t data){
	Wire.beginTransmission(_ADDR);
	Wire.write(static_cast<uint8_t>(regPointer));	// Set register
	uint8_t buffer[] = { static_cast<uint8_t>((data >> 8) & 0xFF),  static_cast<uint8_t>(data & 0xFF)};
	Wire.write(buffer,2);
	Wire.endTransmission();
}

uint16_t INA226::getUInt(){
	Wire.requestFrom(_ADDR, (uint8_t) 2);
	uint8_t MSB = Wire.read();
	uint8_t LSB = Wire.read();
	return (uint16_t) MSB << 8 | (uint16_t) LSB;
}

float INA226::currentLSB() const { return _MAX_CURRENT / 0x8000;}

uint16_t INA226::configRegister_t::serialize() const{
	uint16_t data = reset;
	data <<= 6;
	data |= averaging;
	data <<= 3;
	data |= busConversionTime;
	data <<= 3;
	data |= shuntConversionTime;
	data <<= 3;
	data |= operatingMode;
	return data;
}

void INA226::configRegister_t::deserialize(uint16_t data){
	operatingMode = data;
	shuntConversionTime = (data >>= 3);
	busConversionTime = (data >>= 3);
	averaging = (data >>= 3);
	reset = 0;	// Always zero
}

