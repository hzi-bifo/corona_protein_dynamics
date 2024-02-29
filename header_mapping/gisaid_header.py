import sys
import glob
import os

seq_dir = sys.argv[1]

meta_file = open(sys.argv[2])
meta_lines = meta_file.readlines()

meta_dict = {}
for line in meta_lines[1:]:
	li = line.split("\t")
	meta_dict[li[0]] = (li[2],li[4])

out_dir = sys.argv[3]


for seq_file_name in glob.glob(seq_dir + '/*.fasta'):
  seq_file = open(seq_file_name)
  seq_lines = seq_file.readlines()
  seq_file_name_strip = os.path.basename(seq_file_name)

  out_file = open(out_dir + '/' + seq_file_name_strip,'w')
  for line in seq_lines:
    line = line.replace("\n","")
    if line.startswith(">"):
      l = line.replace(">","")
      try:
        info = meta_dict[l]
        out = ">%s|%s|%s"%(l,info[0],info[1])
        out_file.write("%s\n"%out)
        prnt = True
      except:
        prnt = False
    elif (prnt):
      out_file.write("%s\n"%line)
  out_file.close()
