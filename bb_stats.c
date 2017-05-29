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

int plugin_is_GPL_compatible;

struct bb_plugin_options {
	char *output;
	FILE *output_file;
};

static struct bb_plugin_options *plugin_options;
static struct opt_pass bb_pass = {};

void bb_diagnostic_start(void *data, void *user)
{
	if (plugin_options == NULL)
		return;
	plugin_options->output_file = fopen(plugin_options->output, "w+");
	if (plugin_options->output_file == NULL)
	{
		fprintf(stderr,
		        "bb_diagnostic: Could not open output file: %s\n", 
		        xstrerror(errno));
	}
}

void bb_diagnostic_stop(void *data, void *user)
{
	if (plugin_options != NULL && plugin_options->output_file != NULL)
		fclose(plugin_options->output_file);
	return;
}

static unsigned int bb_pass_execute(void)
{
	printf("Calling my pass: %s\n", plugin_options->output);
	function *func = cfun;
	basic_block bb;

	printf("Function name: %s\n", IDENTIFIER_POINTER(DECL_NAME(func->decl)));

	if (!func->cfg)
	{
		printf("This function has no CFG.\n");
		return 0;
	}

	FOR_EACH_BB_FN(bb, func)
	{
		printf("basic block: %d\n", bb->index);
		rtx insn;
		FOR_BB_INSNS(bb, insn)
		{
			if NONDEBUG_INSN_P(insn)
			{
				printf("len: %d\n", get_attr_length(insn));
			}
		}
	}
	return 0;
}


#if 0
void bb_diagnostic(void *data, void *user)
{	
	struct opt_pass *pass = (struct opt_pass*)data;
	basic_block bb;
	function *func = cfun;
	printf("Current function name: %s\n", current_function_name());
	printf("Pass name: %s\n", pass->name);

	if (!func || !func->cfg)
		return;

	FOR_EACH_BB_FN(bb, func)
	{
		printf("Basic block!\n");
	}

	cgraph_node *node;
	FOR_EACH_FUNCTION(node)
	{
		tree function_decl;
		printf("Function: %s!\n", cgraph_node_name(node));

		tree fndecl = node->symbol.decl;
		function *func = DECL_STRUCT_FUNCTION(fndecl);
		if (!func)
			continue;
		if (!func->cfg)
			continue;
		
		FOR_EACH_BB_FN(bb, func)
		{
			printf("Basic block!\n");
		}
	}
	tree t;
	unsigned i;
	FOR_EACH_VEC_ELT(*all_translation_units, i, t)
	{
		tree type;
		tree chain;
		struct tree_translation_unit_decl *trans = (struct tree_translation_unit_decl*)t;

		chain = TREE_CHAIN(t);

		if (DECL_CONTEXT(t))
		{
			printf("Have a context!\n");
		}

		if (!chain)
		{
			printf("Skipping.\n");
			continue;
		}
		printf("Translation unit: %s\n", trans->language);
		printf("Class: %s\n", TREE_CODE_CLASS_STRING(TREE_CODE(chain)));
		printf("Class: %d\n", tcc_declaration);
	}
	basic_block bb;
	opt_pass *pass = (opt_pass*)data;

	if (!cfun || !cfun->cfg)
		return;

	printf("Start.\n");
	FOR_EACH_BB(bb)
	{
		rtx insn;

		if (!bb->il.x.rtl)
			continue;

		printf("basic block start!\n");
		FOR_BB_INSNS(bb, insn)
		{
			printf("instruction!\n");
		}
		printf("basic block end!\n");
	}

	printf("Done.\n");
	return;
}
#endif

void usage(void)
{
	printf("usage:\n");
}

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
	struct register_pass_info pass_info;

	if (plugin_info->argc != 1)
	{
		usage();
		return 1;
	}

	if (strcmp(plugin_info->argv[0].key, "output"))
	{
		usage();
		return 1;
	}

	plugin_options = (struct bb_plugin_options*)xmalloc(sizeof(
		                struct bb_plugin_options));

	memset(plugin_options, 0, sizeof(struct bb_plugin_options));
	memset(&pass_info, 0, sizeof(struct register_pass_info));

	plugin_options->output = (char *)xmalloc(sizeof(char)*
		                                       strlen(plugin_info->argv[0].value));
	memcpy(plugin_options->output,
		     plugin_info->argv[0].value,
		     strlen(plugin_info->argv[0].value));

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
	                  PLUGIN_ALL_PASSES_START,
										bb_diagnostic_start,
										NULL);
	register_callback(plugin_info->base_name,
	                  PLUGIN_ALL_PASSES_END,
										bb_diagnostic_stop,
										NULL);
	return 0;
}
