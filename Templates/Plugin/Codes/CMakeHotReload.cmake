#make temp dir
if(NOT EXISTS ${tmp})
  file(MAKE_DIRECTORY ${tmp})
endif()

#random name for old dll & pdb
string(RANDOM LENGTH 5 rnd)

#try removing current file.
if(EXISTS ${dll})
  file(REMOVE ${dll})
  if(EXISTS ${dll})
    #or rename - move it.
    file(RENAME ${dll} "${tmp}/${rnd}.dll")
  endif()  
endif()

#all the same for pdb.
if(EXISTS ${pdb})
  file(REMOVE ${pdb})
  if(EXISTS ${pdb})
    file(RENAME ${pdb} "${tmp}/${rnd}.pdb")
  endif()  
endif()