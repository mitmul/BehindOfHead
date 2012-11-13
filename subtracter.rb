# coding: utf-8

$:.unshift "./"

require "fssm"
require "fileutils"
require "opencv"
include OpenCV

Dir.chdir "/Users/saito/Desktop/Hippieb-Vol.4/AutoPicsSubtracter"

FSSM.monitor(".", "**/*") do
  size   = CvSize.new 1440, 900
  images = []

  create do |base, file|
    image = IplImage::load "#{file}"
    image = image.resize size
    images << image
    if images.size == 1
      GUI::Window::destroy_all
      window = GUI::Window.new "atamanoushiro"
      window.resize 1440, 900
      window.show_image image
    end
    if images.size == 2
      subtract = images[0].sub(images[1])
      GUI::Window::destroy_all
      window = GUI::Window.new "atamanoushiro"
      window.resize 1440, 900
      window.show_image subtract
      images = []
    end
    FileUtils.rm "#{file}"
  end
end
