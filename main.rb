# coding: utf-8

$:.unshift "./"

require "fssm"
require "fileutils"

dir = "/Users/saito/Pictures/Eye-Fi/"
dir += Time.now.strftime "%Y-%m-%d"


FSSM.monitor(dir, "**/*") do
  create do |base, file|
    puts "CREATE: #{file}"
    FileUtils.cp "#{base}/#{file}", "ArrivalPics/#{file}"
  end
end
