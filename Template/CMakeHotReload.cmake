message("Hot Reloading !")

string(RANDOM LENGTH 5 fukusima)
message(${fukusima})

#try removing current file.
if(EXISTS ${dll})
  file(REMOVE ${dll})
  if(EXISTS ${dll})
    #or rename it.
    file(RENAME ${dll} "${fukusima}x")
  endif()  
endif()

#all the same for pdb.
if(EXISTS ${pdb})
  file(REMOVE ${pdb})
  if(EXISTS ${pdb})
    file(RENAME ${pdb} "${fukusima}y")
  endif()  
endif()
