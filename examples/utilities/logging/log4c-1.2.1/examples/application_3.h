
typedef struct {
	char* hostname;
	int   pid;
} user_locinfo_t;

#define log4c_category_log_userinfo(a_category, a_void, a_priority, a_format, args...) \
  helper(a_category, __FILE__, __LINE__, __FUNCTION__, a_void, a_priority, a_format , ## args );

static inline void helper(
    const log4c_category_t* a_category,
    char* file,
    int   line,
    const char* func,
    void* user_locinfo,
    int a_priority,
    const char* a_format,
    ...)
{
  log4c_location_info_t locinfo;
  locinfo.loc_file = file;
  locinfo.loc_line = line;
  locinfo.loc_function = func;
  locinfo.loc_data = user_locinfo;
        va_list va;
	        va_start(va, a_format);

  log4c_category_log_locinfo(a_category, &locinfo, a_priority, a_format,va);
       va_end(va);

}

 
