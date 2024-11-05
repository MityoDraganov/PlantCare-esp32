import json

# Path to the JSON file and output header file
json_file_path = "src/config.json"
header_file_path = "src/config.json.h"

# Read JSON data
with open(json_file_path, 'r') as json_file:
    json_data = json.load(json_file)

# Write to header file
with open(header_file_path, 'w') as header_file:
    header_file.write("#pragma once\n\n")
    header_file.write("const char* configJson = R\"rawliteral(\n")
    json_string = json.dumps(json_data, indent=4)
    header_file.write(json_string)
    header_file.write("\n)rawliteral\";\n")
