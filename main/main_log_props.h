#pragma once

#define MAIN_LOG_ENABLE

#ifdef MAIN_LOG_ENABLE
	#ifndef MAIN_LOG_ON
		#define MAIN_LOG_ON
	#endif
#endif

#ifndef MAIN_LOG_ENABLE
	#ifdef MAIN_LOG_ON
		#undef MAIN_LOG_ON
	#endif
#endif