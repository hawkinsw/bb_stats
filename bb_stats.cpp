#include "gcc-plugin.h"
#include "basic-block.h"
#include "rtl.h"
//#include "cgraph.h"
#include "tree.h"
#include "tree-pass.h"
#include "output.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include "options.h"

// Assert that this plugin is GPL compatibly licensed.
int plugin_is_GPL_compatible;

// A data structure for holding the file pointer to
// the file where we will write out information about
// the program's basic blocks.
struct bb_plugin_options {
	FILE *output_file;
};
// Make a global one so that we can use it from the
// different places without worrying about it being
// deallocated.
static struct bb_plugin_options *plugin_options;

// Define a subclass of an rtl optimization pass.
class bb_opt_pass : public rtl_opt_pass {

public:
  bb_opt_pass(const pass_data &data) : rtl_opt_pass(data, nullptr) {}

  // This function is called when more than one
  // of these are needed. Still fuzzy on the details.
  // More later.
	bb_opt_pass *clone() {
    return this;
	}

  // The code that gets executed by the pass
  unsigned int execute(function *func) {
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
      rtx_insn *insn;
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
};

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

int usage(void)
{
	fprintf(stderr, "usage: -fplugin=<path to bb_stats.so>/bb_stats.so\n");
	fprintf(stderr, "      [-fplugin-arg-bb_stats-output=<output filename>]\n");
	return 1;
}

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
	struct register_pass_info pass_info;
	char *filename = NULL;
	struct pass_data bb_pass_data;

	if (plugin_info->argc == 1)
	{
		if (strcmp(plugin_info->argv[0].key, "output"))
			return usage();
		filename = plugin_info->argv[0].value;
	}
	else
	{
		unsigned int filename_alloc_len=(strlen(main_input_filename)+1+3);
		filename_alloc_len *= sizeof(char);

		filename = (char*)xmalloc(filename_alloc_len);
		memset(filename, 0, filename_alloc_len);
		strcpy(filename, main_input_filename);
		strcat(filename, ".bb");

		for (int i = 0; i<strlen(filename); i++)
			if (filename[i] == '/')
				filename[i] = '_';
	}

	plugin_options = (struct bb_plugin_options*)xmalloc(sizeof(
		                struct bb_plugin_options));

	memset(plugin_options, 0, sizeof(struct bb_plugin_options));
	memset(&pass_info, 0, sizeof(struct register_pass_info));
	memset(&bb_pass_data, 0, sizeof(struct pass_data));

	if (bb_initialize_output(filename))
		return 1;

	bb_pass_data.type = RTL_PASS;
	bb_pass_data.name = "bb_plugin";

  bb_opt_pass *bb_pass = new bb_opt_pass(bb_pass_data);

	pass_info.reference_pass_name = "final";
	pass_info.pos_op = PASS_POS_INSERT_AFTER;
	pass_info.pass = bb_pass;

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
