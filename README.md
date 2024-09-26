Future things todo and add : 
- make the core db engine
- implement gui using wXwidgets
- write a network layer which would use socket programming to support LAN queries
- write an upgraded network layer which would support WAN queries maybe using ngrok or custom tunnelling service
- create a react native application so that the software could be used in android devices as well as ios devices
- train an OCR deeplearning model to recognize english and hindi text (devanagari) and also recognize tabular format, which would basically take in the image of tabular data written on a piece of paper and then would run it through the ocr and generate the table itself.
- write drivers that could basically convert a blueDB table to a mysql table , something on this line to make things interchangable between blueDB and mainstream database softwares


Syntax TODO remaining

1) variables and functions syntax 

2) constrainst syntax

3) multi table linking using joins and permutations syntax

Syntax to be parsed 

~ Creating a new table : 

new students :: int id , string name 

~ Adding data to the table : 

add students :: [1 , "Aryan"] , [2 , "Kumar"] , [3 , "Test"]

~ Displaying the data from a table (the outputs would in the normal command line interface , further in the project i would create a gui layer on top of the database
so printing a table would look like an actual table using gui)

print students

print students.name 


print students.name , students.id :: name = "Aryan" || id = 3

~ Removing data from table

remove students ~ would remove every entry from the table

remove students :: name = "Aryan" && id = 1 ~ would remove the entry which has name aryan and id 1


update students :: name = "Aryan" -> name = "Aryan Kumar" , id = 141 ~ would update the record with name aryan to name it aryan kumar and update its id to 141


TODO remaingning
variables and functions syntax 
constrainst syntax
multi table linking using joins and permutations syntax

