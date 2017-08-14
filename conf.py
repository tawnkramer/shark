import sys
import json

def init(config_filename = 'config.json'):
    infile = open(config_filename, "rt")

    #json python module doesn't honor comment lines.
    #so we are going to strip them out.
    json_lines = []
    for line in infile:
        comment = line.find('//')
        if comment == -1:
            json_lines.append(line)
        elif comment > 0:
            remainder = line[:comment]
            json_lines.append(remainder)

    infile.close()
    settings_json = json.loads(''.join(json_lines))

    module = sys.modules[__name__]
    try:
        for name, value in settings_json.iteritems():
            setattr(module, name, value)
    except:
        for name, value in settings_json.items():
            setattr(module, name, value)

