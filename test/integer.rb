require 'hybridsort'
require 'benchmark'

original = Array.new(100000) { |i| i }

Benchmark.bm do |bm|
  for i in 1..27
    prng = Random.new(112234)
    ary = original.shuffle(random: prng)
    bm.report("Threshhold: #{i}") do
      500.times do
        Hybrid.sort!(ary, i)
        ary.shuffle!(random: prng)
      end
    end
  end
end
