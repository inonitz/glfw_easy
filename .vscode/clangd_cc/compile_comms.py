import subprocess as subproc
import numpy as np
import os
import sys
import json




valid_argv1 = ["--help", "-h", "--out"]
valid_argv2 = ["--exec"]
compiler_name_prefix = "x86_64-w64-mingw32-"
compiler_absolute_path = "".join([ "\"" + str(os.environ['MINGW_W64_COMPILER_BASE']) + "\"", "/bin/" ])
valid_compilers = [ "gcc", "g++", "nasm" ]
max_cpp_files = 100
max_c_files   = 100
max_asm_files = 100
verbose = True


# compile command structure that is expected is as follows:
	# [compiler] [arguments] -I[Include Directory]... src_file.ext -o obj_file_ext.o



def sanitizeCommand():
    sys.tracebacklimit = 0
    
    if compiler_absolute_path == "None/bin/": 
        raise NameError("Environment Variable MINGW_W64_COMPILER_BASE undefined. Please define such a variable and re-run the script")
    if len(sys.argv) == 1:
        raise TypeError("Insufficient Args Supplied")
    

    argv1 = str(sys.argv[1])
    if argv1.startswith(valid_argv1[0]) or argv1.startswith(valid_argv1[1]):
        print("Usage: \n    compile_commands.py --out=<Path to resulting json> --exec=<'make script command...'> ")
        exit(0)
    elif not argv1.startswith(valid_argv1[2]):
        raise TypeError("Invalid Argument at Arg[1], given was {}, accepts only: {}".format( argv1, " ".join(valid_argv1) ))

    if len(sys.argv) < 3:
        raise ValueError("--out arg must be followed by --exec arg")


    argv2 = str(sys.argv[2])
    if not argv1.startswith("--out="): raise TypeError("Script must start with output file as first argument.\n") 
    if not argv2.startswith("--exec="): raise TypeError("Scripts' second argument must be a Make Script-Command, e.g. 'make -f ...'\n")


    output_file     = argv1.replace("--out=", "")  # get rid of prepend
    output_filename = output_file.split('/')[-1]
    exec_command    = argv2.replace("--exec=", "").replace("'", "") # get rid of prepend and '
    print("\nGenerating {}... ( {} )".format(output_filename, output_file))
    if verbose:
        print("\n---compile_commands.py begin =>")
        print("sys.argv[2]: make command is: '{}'".format(exec_command))
        print("---compile_commands    end")
	

    tmp = exec_command.split(" ")
    tmp = [s for s in tmp if s != ' ' and s != '']
    if verbose: 
        print("argument list is {}".format(tmp))
    return [output_filename, output_file, tmp]




def parseOutputString(lines):
    parse_strings = [[] * 2] * len(lines)
    # [NOTE]: 
    # Some Lines contain double-quoted strings (because they contain spaces), which I'll call 'tokens' from now on.
    # Since later I have to split each line by spaces between each word, and the tokens hinder this process, I create a list of tokens for each line
    # After the splitting process, I'll replace the tokens by their actual strings 
    c = 0
    ct = 0
    for line in lines:
        token_list = line.split('"')[1::2] # all Double-Quoted strings
        for ti in range(0, len(token_list)):
            line = line.replace(token_list[ti], "token") # replace them by the word 'token'

        line_words = line.split(" ")
        line_words = [word for word in line_words if (word != " " and word != "")] # keep words that are not spaces or empty
        for i in range(0, len(line_words)):
            if "token" in line_words[i]: # return the original values into their respective positions
                line_words[i] = line_words[i].replace("token", token_list[ct])
                ct += 1
        ct = 0

        # compiler_string = line_words[0].split('/')[-1].replace(compiler_name_prefix, "") # find which compiler was used
        # parse_strings[c] = [ line_words, [i for i in range(0, len(valid_compilers)) if valid_compilers[i] == compiler_string] ]
        parse_strings[c] = line_words
        c += 1
    
    return parse_strings




def makeCompileDictionary(sanitizedLines):
    map_str_to_json = \
	{
		"arguments": [],
		"directory": '',
		"file": '',
		"output": ''
	}
    dict_of_line_args = [{} for _ in range(0, len(sanitizedLines))]
    current_path_abs = subproc.run(["pwd"], stdout=subproc.PIPE, shell=True, text=True)
    current_path_abs = current_path_abs.stdout.replace('\n', "")
    if verbose:
        print("abs path found was {}\n\n".format(current_path_abs))


    map_str_to_json["directory"] = current_path_abs
    for i in range(0, len(sanitizedLines)):
        in_file  = sanitizedLines[i][-3]
        out_file = sanitizedLines[i][-1]
        map_str_to_json["arguments"] = sanitizedLines[i]
        map_str_to_json["file"]      = "/".join([current_path_abs, in_file ])
        map_str_to_json["output"]    = "/".join([current_path_abs, out_file])
        dict_of_line_args[i] = map_str_to_json.copy()
    
    if verbose: 
        [print("compile command: {}\n".format(dict)) for dict in dict_of_line_args]
    return dict_of_line_args




def main():
    out_filename, out_fullpath, make_command = sanitizeCommand()
    sp = subproc.run(make_command, stdout=subproc.PIPE, shell=True, text=True, check=True)
    if sp.returncode != 0:
        print("Process was terminated. Error Code {}".format(sp.returncode))
        exit(sp.returncode)
    if verbose:
        print("process ran and retrieved the following output ==>\n\n{}".format(sp.stdout))


    stdout_lines = sp.stdout.split("\n")[4:-4] # get rid of debug & make related lines, also end of compilation status and linking stage output
    stdout_lines = stdout_lines[1::2] # get rid of 'compiling <filetype> <filename> ...', only relevant compile lines
    print("Pre-Sanitized stdout_lines: \nbegin\n{}\nend\n".format(stdout_lines))
    parse_strings = parseOutputString(stdout_lines)
    if verbose:
        print(parse_strings)
    
    
    final_dict = makeCompileDictionary(parse_strings)
    with open(out_fullpath, 'w+') as file:
        json.dump(final_dict, file, indent=4)

    print("[NOTICE]: Created {} file at {}".format(out_filename, out_fullpath))
    exit(0)


if __name__ == "__main__":
    main()