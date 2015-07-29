# What is PipelineCpp ?

It's a C++ framework to implement pipeline/workflows within an application.

When it comes to complexe processes, we tend to see them as a chain of serveral (simpler) operationd.

Unfortunatly, a naive implementation of such a process often leads to a monolithic workflow with little to no flexibility, changing the internal chain of operations leads to horrendous refactoring due to overall complex management of code chunks.
Moreover, it's difficult to switch from a simple to a multi-threaded implementation without heavy refactoring and concerns about concurrency between operations.

PipelineCpp aims to reduce the hassle:

- Build serveral blocks called "Processing Units", you only have to take care of what they expects as input and produce as output.
- Plug your blocks together to make your own pipeline, by pluging a Processing Unit to another one, it creates a queue between them where one is the producer and the other is the consummer, you get to define the type of data being exchanged.
- Feed your pipeline input data and get the output, you can chose the number of working threads to run your Processing Units and push data downstream.
