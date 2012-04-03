require 'hybridsort'
require 'benchmark'

alphabet = %w{ a b c d e f g h i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z}

prng = Random.new()
original = Array.new(10000) do |i|
  str = ""
  prng.rand(1..20).times do
    str << alphabet.sample(random: prng)
  end
  str
end

p original[0]

Benchmark.bm do |bm|
  for i in 1..27
    ary = original.shuffle(random: prng)
    bm.report("Threshhold: #{i}") do
      500.times do
        Hybrid.sort!(ary, i)
        ary.shuffle!(random: prng)
      end
    end
  end
end
