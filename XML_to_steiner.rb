#!/usr/bin/ruby2.3

# This file converts XML description of net into
# special format that is easy to parse from main code.
# Output will contain only numbers (lines with -- is comments):
# -- Grid size --
# min_x
# min_y
# max_x
# max_y
# -- Net --
# number of points
# point_x
# point_y
# -- etc. --

require 'optparse'
require 'pp'
require 'nokogiri'

options = {}
OptionParser.new do |opts|
  opts.on("-i", "--input FILE", "Input XML with net configuration"){ |v| options[:in] = v }
  opts.on("-o", "--output FILE", "Output net configuration file in raw format"){ |v| options[:out] = v }
end.parse!

input = options.fetch(:in)
output = options.fetch(:out)

out = File.open(output, "w")
net = File.open(input) { |f| Nokogiri::XML(f) }
task = net.child
fail "Task node is missed" unless task.name == 'root'
task_children = task.children.select{ |c| c.element? }
fail "Too many child nodes in task" unless task_children.size == 2
fail "Grid node should be first" unless task_children[0].name == "grid"
fail "Net node should be second" unless task_children[1].name == "net"

grid = net.xpath('///grid').first
fail "Malformed grid node" unless grid.children.empty?

grid_params = grid.attributes
out.puts(grid_params["min_x"].value)
out.puts(grid_params["min_y"].value)
out.puts(grid_params["max_x"].value)
out.puts(grid_params["max_y"].value)

points = net.xpath('///point')
out.puts(points.size)
points.each do |pt|
  attrs = pt.attributes
  fail "Bad point layer" unless attrs["layer"].value == "pins"
  fail "Bad point type" unless attrs["type"].value == "pin"
  out.puts(attrs["x"].value)
  out.puts(attrs["y"].value)
end

out.close()

