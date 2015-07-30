#include <iostream>
#include <PipelineCpp.h>

class DummyBase{

    public:
    DummyBase(){}
    ~DummyBase(){}

};

class DummyDerived: public DummyBase{

    public:
    DummyDerived(){}
    ~DummyDerived(){}

    virtual void execute(){}

};

class Addition: public PipelineCpp::ProcessingUnit{

    public:
    Addition(){}
    ~Addition(){}

    virtual void execute(){

        std::cout << "Addition" << std::endl;

        float op0 = popIn<float>(0);
        float op1 = popIn<float>(1);

        pushOut<float>(0, op0+op1);

    }

};

class Subtraction: public PipelineCpp::ProcessingUnit{

    public:
    Subtraction(){}
    ~Subtraction(){}

    virtual void execute(){

      std::cout << "Subtraction" << std::endl;

      float op0 = popIn<float>(0);
      float op1 = popIn<float>(1);

      pushOut<float>(0, op0-op1);

    }

};

class Mutliplication: public PipelineCpp::ProcessingUnit{

    public:
    Mutliplication(){}
    ~Mutliplication(){}

    virtual void execute(){

      std::cout << "Mutliplication" << std::endl;

      float op0 = popIn<float>(0);
      float op1 = popIn<float>(1);

      pushOut<float>(0, op0*op1);

    }

};

class Division: public PipelineCpp::ProcessingUnit{

    public:
    Division(){}
    ~Division(){}

    virtual void execute(){

      std::cout << "Division" << std::endl;

      float op0 = popIn<float>(0);
      float op1 = popIn<float>(1);

      pushOut<float>(0, op0/op1);

    }

};

class Duplication: public PipelineCpp::ProcessingUnit{

    public:
    Duplication(){}
    ~Duplication(){}

    virtual void execute(){

      std::cout << "Duplication" << std::endl;

      float op0 = popIn<float>(0);

      pushOut<float>(0, op0);
      pushOut<float>(1, op0);

    }

};

int main(int argc, char* argv[])
{

    PipelineCpp::ConcreteQueue<int> intQueue(3);

    intQueue.push(5);
    intQueue.push(3);
    intQueue.push(2);

    std::cout << intQueue.pop() << std::endl;
    std::cout << intQueue.pop() << std::endl;
    std::cout << intQueue.pop() << std::endl;

    PipelineCpp::Pipeline<float,float> pipeline(10);

    try{
      //PipelineCpp::PToken pu_add = pipeline.create<Addition>();
	PipelineCpp::PToken pu_sub = pipeline.create<Subtraction>();	
	PipelineCpp::PToken pu_mul = pipeline.create<Mutliplication>();
	PipelineCpp::PToken pu_div = pipeline.create<Division>();
	PipelineCpp::PToken pu_dup1 = pipeline.create<Duplication>();
	PipelineCpp::PToken pu_dup2 = pipeline.create<Duplication>();
	PipelineCpp::PToken pu_dup3 = pipeline.create<Duplication>();
	pipeline.plugInput(pu_dup1);
	pipeline.plug<float>(pu_dup1, pu_dup2);
	pipeline.plug<float>(pu_dup1, pu_dup3);
	
	pipeline.plug<float>(pu_dup2, pu_mul);
	pipeline.plug<float>(pu_dup2, pu_mul);

	pipeline.plug<float>(pu_dup3, pu_div);
	pipeline.plug<float>(pu_dup3, pu_div);

	pipeline.plug<float>(pu_mul, pu_sub);
	pipeline.plug<float>(pu_div, pu_sub);
	pipeline.plugOutput(pu_sub);
    }catch(PipelineCpp::PipelineException e){std::cout << e.what() << std::endl;}

    std::cout << "pipeline_built" << std::endl;

    std::cout << pipeline.feed(10) << std::endl;

    return 0;

}

