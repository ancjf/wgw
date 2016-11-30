
#include <vector>
using namespace std;

void HIDSampleFunc();
HANDLE OpenMyHIDDevice(int overlapped);
void HIDSearch(vector<CString> &vstr);
