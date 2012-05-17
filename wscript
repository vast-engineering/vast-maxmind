import Options
from os import unlink, symlink, popen
from os.path import exists 

srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  conf.check( header_name='GeoIP.h', 
              mandatory = True,
	          errmsg = "GeoIP headers not found.  Please install package - http://www.maxmind.com/app/c")
  conf.check( header_name='GeoIPCity.h', 
              mandatory = True,
	          errmsg = "GeoIPCity headers not found.  Please install package - http://www.maxmind.com/app/c")
  conf.env['LIB_MAXMINDGEO'] = ['GeoIP']



def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "vastmaxmind"
  obj.source = "vastmaxmind.cc"
  obj.cxxflags = ["-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE"]
  obj.uselib = "MAXMINDGEO"

def shutdown():
  if Options.commands['clean']:
    if exists('vastmaxmind.node'): unlink('vastmaxmind.node')
  else:
    if exists('build/default/vastmaxmind.node') and not exists('vastmaxmind.node'):
      symlink('build/default/vastmaxmind.node', 'vastmaxmind.node')
