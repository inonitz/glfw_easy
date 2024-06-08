import subprocess as subproc
import numpy as np
import os
import sys
import json




valid_argv1 = ["--help", "-h", "--out"]
valid_argv2 = ["--exec"]
compiler_name_prefix = "x86_64-w64-mingw32-"
compiler_absolute_path = "".join([ "\"" + str(os.environ['COMPILER_BASE_FOLDER']) + "\"", "/bin/" ])
valid_compilers = [ "gcc", "g++", "nasm" ]
max_cpp_files = 100
max_c_files   = 100
max_asm_files = 100
# verbose = True
verbose = False


# compile command structure that is expected is as follows:
	# [compiler] [arguments] -I[Include Directory]... src_file.ext -o obj_file_ext.o



def sanitizeCommand():
    sys.tracebacklimit = 0
    
    if compiler_absolute_path == "None/bin/": 
        raise NameError("Environment Variable COMPILER_BASE_FOLDER undefined. Please define such a variable and re-run the script")
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
    print("\n[compile_commands.py][Generating] {} ... [{}]".format(output_filename, output_file))
    if verbose:
        print("[compile_commands.py][ sys.argv[2] ] ""{}"" ".format(exec_command))
	

    tmp = exec_command.split(" ")
    tmp = [s for s in tmp if s != ' ' and s != '']
    if verbose: 
        print("[compile_commands.py][Sanitized_Command][ ", end="")
        [print("{} , ".format(arg), end="") for arg in tmp]
        print("\b\b]")
    return [output_filename, output_file, tmp]




def parseOutputStringDebug(lines):
    parse_strings = [[] * 2] * len(lines)
    # [NOTE]: 
    # Some Lines contain double-quoted strings (because they contain spaces), which I'll call 'tokens' from now on.
    # Since later I have to split each line by spaces between each word, and the tokens hinder this process, I create a list of tokens for each line
    # After the splitting process, I'll replace the tokens by their actual strings 
    c = 0
    ct = 0
    for line in lines:
        # print("Line\n  Line (Before) ==> {}".format(line))
        token_list = line.split('"')[1::2] # all Double-Quoted strings
        for ti in range(0, len(token_list)):
            line = line.replace(token_list[ti], "token") # replace them by the word 'token'

        # print("  Line  (After) ==> {}".format(line))
        line_words = line.split(" ")
        line_words = [word for word in line_words if (word != " " and word != "")] # keep words that are not spaces or empty
        # print("  Word List (Before) {\n", end="")
        # [ print("    {}".format(word)) for word in line_words ]
        # print("  }")
        # print("  Token List {\n", end="")
        # [ print("    {}".format(token)) for token in token_list ]
        # print("  }")
        for i in range(0, len(line_words)):
            if "token" in line_words[i]: # return the original values into their respective positions
                line_words[i] = line_words[i].replace('"token"', token_list[ct]) # This Makes sure the Double Quotes are also replaced, besides the token itself
                ct += 1
        ct = 0

        # print("  Word List (After) {\n", end="")
        # [ print("    {}".format(word)) for word in line_words ]
        # print("  }")

        parse_strings[c] = line_words[1::]
        c += 1


    return parse_strings

def parseOutputStrings(lines):
    parse_strings = [[] * 2] * len(lines)
    c = 0
    ct = 0
    for line in lines:
        token_list = line.split('"')[1::2]
        for ti in range(0, len(token_list)):
            line = line.replace(token_list[ti], "token")

        line_words = line.split(" ")
        line_words = [word for word in line_words if (word != " " and word != "")]
        for i in range(0, len(line_words)):
            if "token" in line_words[i]:
                line_words[i] = line_words[i].replace('"token"', token_list[ct])
                ct += 1
        ct = 0

        parse_strings[c] = line_words[1::]
        c += 1
    return parse_strings




def makeCompileDictionary(sanitizedLines):
    map_str_to_json = \
	{
		"directory": '',
		"file": '',
		"output": '',
		"arguments": []
	}
    dict_of_line_args = [{} for _ in range(0, len(sanitizedLines))]
    current_path_abs = subproc.run(["pwd"], stdout=subproc.PIPE, shell=True, text=True) 
    current_path_abs = "cygpath -m {}".format( current_path_abs.stdout.replace('\n', "") )
    current_path_abs = subproc.run(current_path_abs, stdout=subproc.PIPE, shell=True, text=True) # Will probably cause problems on linux/mac
    current_path_abs = current_path_abs.stdout.replace('\n', "")
    if verbose:
        print("[compile_commands.py][makeCompileDictionary] PATH_ABS=""{}""\n".format(current_path_abs))


    map_str_to_json["directory"] = current_path_abs
    for i in range(0, len(sanitizedLines)):
        in_file  = sanitizedLines[i][-3]
        out_file = sanitizedLines[i][-1]
        map_str_to_json["arguments"] = sanitizedLines[i]
        map_str_to_json["file"]      = "/".join([current_path_abs, in_file ])
        map_str_to_json["output"]    = "/".join([current_path_abs, out_file])
        dict_of_line_args[i] = map_str_to_json.copy()
    
    if verbose: 
        print("Compilation Dictionary:\nBegin")
        for dictio in dict_of_line_args:
            print("  Command Dictionary =>")
            [ print( "    [{}]: '{}'".format(key, value) ) for key, value in dictio.items() ]
        print("End\n\n\n")
    return dict_of_line_args




def main():
    out_filename, out_fullpath, make_command = sanitizeCommand()
    sp = subproc.run(make_command, stdout=subproc.PIPE, stderr=subproc.PIPE, shell=True, text=True, check=False)
    if sp.returncode != 2:
        print("Process was terminated. Error Code {}".format(sp.returncode))
        exit(sp.returncode)
    if verbose:
        print("[compile_commands.py][SubProcess][Exit=2] Returned:\nBegin\n\n\n{}\n\nEnd".format(sp.stdout))


    stdout_lines = sp.stdout.split("\n")
    compile_indices = []
    lines_filtered = []
    for i in range(0, len(stdout_lines) ):
        if stdout_lines[i].startswith("[COMPILE]"):
            compile_indices.append(i)
    lines_filtered = [ stdout_lines[i] for i in compile_indices ]
    if verbose:
        print("\n\nPre-Sanitized stdout_lines: \nBegin")
        [ print(lines_filtered[i]) for i in range(0, len(lines_filtered)) ]
        print("End\n")


    parse_strings = parseOutputStrings(lines_filtered)
    if verbose:
        print("\nParsed Strings:\nBegin")
        [ print(parsed) for parsed in parse_strings]
        print("End")


    final_dict = makeCompileDictionary(parse_strings)
    with open(out_fullpath, 'w+') as out_file:
        tmp = json.dumps(final_dict, indent=4)
        out_file.write(tmp)

    print("[NOTICE]: Created {} file at {}".format(out_filename, out_fullpath))
    exit(0)


if __name__ == "__main__":
    main()