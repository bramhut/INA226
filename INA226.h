#ifndef INA226_h
#define INA226_h

#include <Arduino.h>
#include <Wire.h>

#define INA226_ENABLE_DEBUG

// ENUMS
enum class convTime_t : uint8_t{
	T_140US = 0,
	T_204US = 1,
	T_332US = 2,
	T_588US = 3,
	T_1100US = 4,
	T_2116US = 5,
	T_4156US = 6,
	T_8244US = 7
};

enum class avg_t : uint8_t{
	A_1 = 0,
	A_4 = 1,
	A_16 = 2,
	A_64 = 3,
	A_128 = 4,
	A_256 = 5,
	A_512 = 6,
	A_1024 = 7
};

class INA226{
public:

	// CONSTRUCTOR & BEGIN
	INA226(uint8_t SDA, uint8_t SCL, float MAX_CURRENT, float SHUNT_RES, uint8_t ADDR = 0x40);
	bool begin();

	// GETTERS
	float getPower();
	float getBusVoltage();
	float getCurrent();
	float getShuntVoltage();

	// SETTERS
	void setBusConversionTime(convTime_t time);
	void setShuntConversionTime(convTime_t time);
	void setAveraging(avg_t avg);
	
private:

	uint8_t _SDA;
	uint8_t _SCL;
	uint8_t _ADDR;
	float _MAX_CURRENT;
	float _SHUNT_RES;

	enum class register_t : uint8_t{
		CONFIGURATION	= 0,
		SHUNT_V			= 1,
		BUS_V			= 2,
		POWER			= 3,
		CURRENT			= 4,
		CALIBRATION		= 5,
		MASK_ENABLE		= 6,
		ALERT_LIMIT		= 7,
		NOT_SET			= 8
	}regPointer = register_t::NOT_SET;

	struct configRegister_t{
		bool reset : 1;
		uint8_t averaging : 3;
		uint8_t busConversionTime : 3;
		uint8_t shuntConversionTime : 3;
		uint8_t operatingMode : 3;

		uint16_t serialize() const;
		void deserialize(uint16_t data);
	}config;

	void setRegister(const register_t reg);
	int16_t getInt();
	uint16_t getUInt();
	void writeUInt(uint16_t data);
	void setConfig();
	void getConfig();
	void setMaxCurrent();

	float currentLSB() const;
};

#endif