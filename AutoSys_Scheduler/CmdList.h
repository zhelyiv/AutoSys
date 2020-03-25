#pragma once
#include "CmdHelpers.h"

/////////////////////////////////////////////////////////////////////////////////////
// list of all commands supported over tcp conn

enum APP_COMMANDS
{
	cmd_empty = 0
	, cmd_get_command_list
	, cmd_execute 
	, cmd_get_status
	, cmd_start_job
};

/////////////////////////////////////////////////////////////////////////////////////
// command details 

CMD_DETAILS const g_CommandNames[] =
{
	{ APP_COMMANDS::cmd_empty , "empty", "" }
	//
	,{ APP_COMMANDS::cmd_get_command_list , "Get command list", "" }
	// 
	,{ APP_COMMANDS::cmd_execute , "Execute command script",
	"<c>"
	"<params>"
	"<cmd></cmd>"
	"<script>"
		CLRF
		" action : new							" CLRF
		" name : test.job.lurasys;				" CLRF
		" description: test meta script;		" CLRF
		" type: c,e,p,v;						" CLRF
		" priority : 1to3;						" CLRF
		" status : 1to7;						" CLRF
		" status_comment: free text;			" CLRF
		" command_line: notepad;				" CLRF
		" command_params: params;				" CLRF
		" shedule: d;							" CLRF
		" days_of_week: mo tu we th fr sa su;	" CLRF
		" start_dt_unix:YYYYDDMMHHMMSS;			" CLRF
		" expiry_dt_unix:YYYYDDMMHHMMSS;		" CLRF
		" stdout_file: stdout_notepadjob.txt;	" CLRF
		" stderr_file: stderr_notepadjob.txt;	" CLRF
		" comment: free text;					" CLRF
		" timeout_seconds : 9999;				" CLRF
		CLRF
	"</script>"
	"</params>"
	"</c>"
	}
	//
	,{ APP_COMMANDS::cmd_get_status , "Get job status",
	"<c>"
	"<params>"
	"<cmd></cmd>"
	"<ids></ids>"
	"</params>"
	"</c>"
	}
	//
	,{ APP_COMMANDS::cmd_start_job , "Start job",
	"<c>"
	"<params>"
	"<cmd></cmd>"
	"<ids></ids>"
	"</params>"
	"</c>"
	}
	//
};