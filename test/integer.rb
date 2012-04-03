require 'hybridsort'
require 'benchmark'

original = Array.new(10000) { |i| i }.shuffle!

Benchmark.bm do |bm|
  for i in 1..25
    bm.report("Threshhold: #{i}") { 1000.times { Hybrid.sort(original, i) } }
  end
end
