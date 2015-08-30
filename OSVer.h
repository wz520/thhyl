#ifndef __OSVER_H_522BFEBA_
#     define __OSVER_H_522BFEBA_

// NOTE: OSVerInit() must be called before using other functions

void OSVerInit();
BOOL OSVerAboveXP_SP2();

// NOTE: returns static string, "Windows" is not included
LPCTSTR OSVerGetOSString();

// NOTE: returns static string
LPCTSTR OSVerGetCSDVersion();

#endif /* __OSVER_H_522BFEBA_ */
