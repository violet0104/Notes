class Man:
    def __init__(self, name):
        self.name = name
        print("Initialized!")

    def hello(self):
        print("Hello " + self.name + "!")

    def goodbye(self):
        print("GoodBye " + self.name + "!")

m = Man("violet")
m.hello()
m.goodbye()