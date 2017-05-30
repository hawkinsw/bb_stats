#include "gcc-plugin.h"
#include "basic-block.h"
#include "rtl.h"
#include "cgraph.h"
#include "tree.h"
#include "tree-pass.h"
#include "output.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

int plugin_is_GPL_compatible;

struct bb_plugin_options {
	FILE *output_file;
};

static struct bb_plugin_options *plugin_options;
static struct opt_pass bb_pass;

int bb_initialize_output(char *filename)
{
	if (plugin_options == NULL)
	{
		fprintf(stderr,
		        "bb_diagnostic: Options structure not allocated.\n");
		return 1;
	}

	if (!(plugin_options->output_file = fopen(filename, "w+")))
	{
		fprintf(stderr,
		        "bb_diagnostic: Could not open output file: %s\n", 
		        xstrerror(errno));
		return 1;
	}
	return 0;
}

void bb_diagnostic_stop(void *data, void *user)
{
	if (plugin_options != NULL && plugin_options->output_file != NULL)
		fclose(plugin_options->output_file);
	return;
}

static unsigned int bb_pass_execute(void)
{
	function *func = cfun;
	basic_block bb;
	FILE *output_file = plugin_options->output_file;
	unsigned int function_size = 0;

	fprintf(output_file,
	        "Function: %s\n", IDENTIFIER_POINTER(DECL_NAME(func->decl)));

	if (!func->cfg)
		return 0;

	FOR_EACH_BB_FN(bb, func)
	{
		unsigned int bb_size = 0;
		rtx insn;
		fprintf(output_file,
		        "bb %d, ", bb->index);

		FOR_BB_INSNS(bb, insn)
		{
			if NONDEBUG_INSN_P(insn)
			{
				/*
				 * It is possible that get_attr_length() returns
				 * 0. This is the case when the target does not
				 * need to generate any instructions for a
				 * particular RTL.
				 */
				bb_size+=get_attr_length(insn);
			}
		}
		fprintf(output_file,
		        "%d\n", bb_size);
		function_size += bb_size;
	}
	fprintf(output_file,
	        "function, %d\n", function_size);
	return 0;
}

int usage(void)
{
	fprintf(stderr, "usage: -fplugin=<path to bb_stats.so>/bb_stats.so\n");
	fprintf(stderr, "       -fplugin-arg-bb_stats-output=<output filename>\n");
	return 1;
}

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
	struct register_pass_info pass_info;

	if (plugin_info->argc != 1)
		return usage();

	if (strcmp(plugin_info->argv[0].key, "output"))
		return usage();

	plugin_options = (struct bb_plugin_options*)xmalloc(sizeof(
		                struct bb_plugin_options));

	memset(plugin_options, 0, sizeof(struct bb_plugin_options));
	memset(&pass_info, 0, sizeof(struct register_pass_info));

	if (bb_initialize_output(plugin_info->argv[0].value))
		return 1;

	pass_info.reference_pass_name = "final";
	pass_info.pos_op = PASS_POS_INSERT_AFTER;
	pass_info.pass = &bb_pass;

	bb_pass.type = RTL_PASS;
	bb_pass.name = "bb_plugin";
	bb_pass.execute = bb_pass_execute;

	register_callback(plugin_info->base_name,
	                  PLUGIN_PASS_MANAGER_SETUP,
	                  NULL,
	                  &pass_info);

	register_callback(plugin_info->base_name,
	                  PLUGIN_FINISH,
	                  bb_diagnostic_stop,
	                  NULL);
	return 0;
}
