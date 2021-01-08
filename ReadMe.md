# Akinator

> What can he do?
>
> Here's the main menu :
>
> ```
>              MAIN MENU                 
> data: %s
> 
> Start game(you shoud make a word)  : [G]
> Print definition                   : [D]
> Compare two words                  : [C]
> Save data                          : [S]
> Open akinator's tree(only with dot): [O]
> Load another data                  : [N]
> Exit                               : [X]
> Turn on/off voice                  : [T]
> ```
>
> Before starting game make sure that Data.txt (or your standard data file) consist of at least one definition in quotes 
>
> #### [G] Game
>
> >You should make a word and answer questions about it.
> >
> >In the end Akinator will try to guess your word.
> >
> >Sometimes it won't be able to guess correctly. In this case you should supplement database with this word:
> >
> >- Akinator will ask you to type your word.
> >- Then he will ask you to enter the difference between your word and the one he suggested.
> >- You should finish sentence "**Your word** (is) ...".
> >
> >***But*** if Akinator already know this word with another definition, it will tell you about it and won't add
> >
> >your word to database twice.
>
> #### [D] Print definition
>
> >You should make a word and type it.
> >
> >If Akinator know this word, it will write his definition.
>
> #### [C] Compare two words
>
> >You should make two words and type it.
> >
> >If Akinator know both this words, it will build a sentence, which will be consist of similarities and differences of your words.
>
> #### [S] Save data
>
> >If Akinator's tree wasn't changed, nothing will happen
> >
> >If it was changed, this tree will be saved as main one, and if you will exit and load this database again,
> >
> >Akinator will already know all words you added in previos session
>
> #### [O] Open akinator's tree
>
> >If you have dot on your PC, will be created, saved and opened picture with tree visualization
>
> #### [N] Load another data
>
> >If you have another database, you can load it with this option
> >
> >All unsaved changes will be lost
>
> #### [T] Turn on/off voice
>
> >If you have PowerShell you can turn on voice, and Akinator will speak everything what he print
> >
> >But sometimes it is too slow
>
> #### [X] Exit
>
> >Finishing work with Akinator
> >
> >All unsaved changes will be lost

