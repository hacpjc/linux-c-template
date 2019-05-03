#ifndef SRC_STDMSG_STDMSG_H_
#define SRC_STDMSG_STDMSG_H_

typedef enum
{
	STDMSG_LV_NONE = 0,
	STDMSG_LV_ERR,
	STDMSG_LV_INFO,
#define STDMSG_LV_DFL STDMSG_LV_INFO
	STDMSG_LV_DBG,
	STDMSG_LV_VBS,
	STDMSG_LV_MAX
} stdmsg_level_t;

extern int stdmsg_lv;

static inline __attribute__((unused)) void stdmsg_lv_inc(void)
{
	if (stdmsg_lv < STDMSG_LV_MAX)
	{
		stdmsg_lv++;
	}
}

static inline __attribute__((unused)) void stdmsg_lv_dec(void)
{
	if (stdmsg_lv > STDMSG_LV_NONE)
	{
		stdmsg_lv--;
	}
}

static inline __attribute__((unused)) void stdmsg_lv_set(const stdmsg_level_t lv)
{
	if (lv >= STDMSG_LV_MAX)
	{
		stdmsg_lv = STDMSG_LV_MAX;
	}
	else
	{
		stdmsg_lv = lv;
	}
}

static inline __attribute__((unused)) stdmsg_level_t stdmsg_lv_get(void)
{
	return stdmsg_lv;
}

#define stdmsg_lv_test(_lv) ((_lv <= stdmsg_lv) ? 1 : 0)

#if defined(STDMSG_DEBUG_ENABLE) || (1) //!< Plz say 1 to activate
#define DBG(fmt, args...) do { if (stdmsg_lv_test(STDMSG_LV_DBG)) { printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##args); } } while (0)
#define DBG_RAW(fmt, args...) do { if (stdmsg_lv_test(STDMSG_LV_DBG)) { printf(fmt, ##args); } } while (0)
#define VBS(fmt, args...) do { if (stdmsg_lv_test(STDMSG_LV_VBS)) { printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ##args); } } while (0)
#define VBS_RAW(fmt, args...) do { if (stdmsg_lv_test(STDMSG_LV_VBS)) { printf(fmt, ##args); } } while (0)
#else
#define DBG(fmt, args...) do { if (0) { printf(fmt, ##args); } } while (0)
#define DBG_RAW(fmt, args...) do { if (0) { printf(fmt, ##args); } } while (0)
#define VBS(fmt, args...) do { if (0) { printf(fmt, ##args); } } while (0)
#define VBS_RAW(fmt, args...) do { if (0) { printf(fmt, ##args); } } while (0)
#endif

#define STDMSG_ERR_PFX " * ERROR: "
#define ERR(fmt, args...) do { if (stdmsg_lv_test(STDMSG_LV_ERR)) { fprintf(stderr, STDMSG_ERR_PFX fmt "\n", ##args); } } while (0)
#define ERR_RAW(fmt, args...) do { if (stdmsg_lv_test(STDMSG_LV_ERR)) { fprintf(stderr, fmt, ##args); } } while (0)

#define INFO(fmt, args...) do { if (stdmsg_lv_test(STDMSG_LV_INFO)) { printf(fmt "\n", ##args); } } while (0)
#define INFO_RAW(fmt, args...) do { if (stdmsg_lv_test(STDMSG_LV_INFO)) { printf(fmt, ##args); } } while (0)

#endif /* SRC_STDMSG_STDMSG_H_ */
