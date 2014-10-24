#ifndef _PROPERTY_H_
#define _PROPERTY_H_

#include <string>
#include <cutils/properties.h>

class PropertyManager {
public:
	PropertyManager();
	virtual ~PropertyManager();
	std::string GetProperty(std::string key, std::string defaultValue);
};

#endif
