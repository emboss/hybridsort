require 'hybridsort'
require 'benchmark'

alphabet = %w{ a b c d e f g h i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z }

prng = Random.new
original = Array.new(1000) do |i|
  str = ""
  prng.rand(1..15).times { str << alphabet.sample(random: prng) }
  str.to_sym
end

Benchmark.bm do |bm|
  for i in 1..25
    bm.report("Threshhold: #{i}") { 1000.times { Hybrid.sort(original, i) } }
  end
end
