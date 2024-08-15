from os.path import join, dirname

Import("env")

def before_build_source(*args, **kwargs):
    # Building SPIFFS filesystem image
    spiffs_dir = join(dirname(__file__), 'data')
    env.Execute("python3 -m platformio run --target buildfs")

env.AddPreAction("buildfs", before_build_source)
