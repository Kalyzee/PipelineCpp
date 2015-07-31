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
    Addition():PipelineCpp::ProcessingUnit("Addition")
    {
        addInType<float>();
        addInType<float>();
        addOutType<float>();
    }
    ~Addition(){}

    virtual void execute(){

        std::cout << _name << std::endl;

        float op0 = popIn<float>(0);
        float op1 = popIn<float>(1);

        pushOut<float>(0, op0+op1);

    }

};

class Subtraction: public PipelineCpp::ProcessingUnit{

    public:
    Subtraction():PipelineCpp::ProcessingUnit("Subtraction")
    {
        addInType<float>();
        addInType<float>();
        addOutType<float>();
    }
    ~Subtraction(){}

    virtual void execute(){

      std::cout << _name << std::endl;

      float op0 = popIn<float>(0);
      float op1 = popIn<float>(1);

      pushOut<float>(0, op0-op1);

    }

};

class Mutliplication: public PipelineCpp::ProcessingUnit{

    public:
    Mutliplication():PipelineCpp::ProcessingUnit("Mutliplication")
    {
        addInType<float>();
        addInType<float>();
        addOutType<float>();
    }
    ~Mutliplication(){}

    virtual void execute(){

      std::cout << _name << std::endl;

      float op0 = popIn<float>(0);
      float op1 = popIn<float>(1);

      pushOut<float>(0, op0*op1);

    }

};

class Division: public PipelineCpp::ProcessingUnit{

    public:
    Division():PipelineCpp::ProcessingUnit("Division")
    {
        addInType<float>();
        addInType<float>();
        addOutType<float>();
    }
    ~Division(){}

    virtual void execute(){

      std::cout << _name << std::endl;

      float op0 = popIn<float>(0);
      float op1 = popIn<float>(1);

      pushOut<float>(0, op0/op1);

    }

};

class Duplication: public PipelineCpp::ProcessingUnit{

    public:
    Duplication():PipelineCpp::ProcessingUnit("Duplication")
    {
        addInType<float>();
        addOutType<float>();
        addOutType<float>();
    }
    ~Duplication(){}

    virtual void execute(){

      std::cout << _name << std::endl;

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
        std::cout << "pu_sub" << std::endl;
        PipelineCpp::PToken pu_sub = pipeline.create<Subtraction>();
        std::cout << "pu_mul" << std::endl;
        PipelineCpp::PToken pu_mul = pipeline.create<Mutliplication>();
        std::cout << "pu_div" << std::endl;
        PipelineCpp::PToken pu_div = pipeline.create<Division>();
        std::cout << "pu_dup1" << std::endl;
        PipelineCpp::PToken pu_dup1 = pipeline.create<Duplication>();
        std::cout << "pu_dup2" << std::endl;
        PipelineCpp::PToken pu_dup2 = pipeline.create<Duplication>();
        std::cout << "pu_dup3" << std::endl;
        PipelineCpp::PToken pu_dup3 = pipeline.create<Duplication>();
        
        std::cout << "plug1" << std::endl;
        pipeline.plugInput(pu_dup1);
        std::cout << "plug2" << std::endl;
        pipeline.plug<float>(pu_dup1, 0, pu_dup2, 0);
        std::cout << "plug3" << std::endl;
        pipeline.plug<float>(pu_dup1, 1, pu_dup3, 0);
        
        std::cout << "plug4" << std::endl;
        pipeline.plug<float>(pu_dup2, 0, pu_mul, 0);
        std::cout << "plug5" << std::endl;
        pipeline.plug<float>(pu_dup2, 0, pu_mul, 1);

        std::cout << "plug6" << std::endl;
        pipeline.plug<float>(pu_dup3, 0, pu_div, 0);
        std::cout << "plug7" << std::endl;
        pipeline.plug<float>(pu_dup3, 1, pu_div, 1);

        std::cout << "plug8" << std::endl;
        pipeline.plug<float>(pu_mul, 0, pu_sub, 0);
        std::cout << "plug9" << std::endl;
        pipeline.plug<float>(pu_div, 0, pu_sub, 1);
        std::cout << "plug10" << std::endl;
        pipeline.plugOutput(pu_sub);
    }catch(PipelineCpp::PipelineException e){std::cout << e.what() << std::endl;}

    std::cout << "pipeline_built" << std::endl;

    std::cout << pipeline.feed(10) << std::endl;
    std::cout << pipeline.feed(100) << std::endl;

    return 0;

}

