class MultiPlayer:
    def __init__(self,game):
        self.game = game
        self.x=0

    def addBuilding(self,type,pos):
        self.x=1
        self.game.add_building(pos[0]+10,pos[1]+10,type)
        self.x=0
        
        