require 'rake'
require 'rake/extensiontask'

task :default => :compile

Rake::ExtensionTask.new('hybridsort') do |ext|
  ext.ext_dir = "ext"
  ext.lib_dir = "lib"
end

task :build => :compile
