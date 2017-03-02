// XT headers
#include <xt/time.h>

void xtTimestampNow(struct xtTimestamp *timestamp)
{
	struct xtTimestamp now;
	xtTimestampReal(&now);
	int gmtOffset;
	int ret = xtCalendarGetGMTOffset(&gmtOffset);
	if (ret != 0)
		goto error;
	bool isDST;
	if (xtCalendarIsDST(&isDST) != 0)
		goto error;
	if (isDST)
		now.sec += 3600LLU; // Advance by 1 hour if we're in DST
	now.sec += gmtOffset * 60;
	timestamp->sec = now.sec;
	timestamp->nsec = now.nsec;
	return;
error:
	timestamp->sec = 0;
	timestamp->nsec = 0;
}

unsigned long long xtTimestampToMS(struct xtTimestamp *timestamp)
{
	return timestamp->sec * 1000LLU + timestamp->nsec / 1000000LLU;
}

unsigned long long xtTimestampToUS(struct xtTimestamp *timestamp)
{
	return timestamp->sec * 1000000LLU + timestamp->nsec / 1000LLU;
}
