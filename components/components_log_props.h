#pragma once

//#define COMPONENTS_LOG_ENABLE

#ifdef COMPONENTS_LOG_ENABLE
	#ifndef COMPONENTS_LOG_ON
		#define COMPONENTS_LOG_ON
	#endif
#endif

#ifndef COMPONENTS_LOG_ENABLE
	#ifdef COMPONENTS_LOG_ON
		#undef COMPONENTS_LOG_ON
	#endif
#endif