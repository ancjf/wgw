
#include <vector>
using namespace std;

void HIDSampleFunc();
HANDLE OpenMyHIDDevice(int overlapped);
void HIDSearch(vector<CString> &vstr);

int HIDStartRead(struct readThreadData *threaddata);
int HIDWrite(HANDLE hand, const char *buf, unsigned len, unsigned timeout);

int HIDOpen(CString name, unsigned speed, struct readThreadData *data);

int HIDClose(struct readThreadData *data);
