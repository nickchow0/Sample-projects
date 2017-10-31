from random import shuffle

# this class defines a card and its methods 
class Card:
    def __init__(self, symbol, value):
        self.symbol = symbol
        self.value = value
    
    def __cmp__(self, other):
        if self.value < other.value:
            return -1
        elif self.value == other.value:
            return 0
        return 1
        
    def __eq__(self, other):
        return ((self.symbol, self.value) == (other.symbol, other.value))
    
    def __ne__(self, other):
        return not self == other
    
    def __str__(self):
        text = ""
        if self.value < 0:
            return "Joker";
        elif self.value == 11:
            text = "J"
        elif self.value == 12:
            text = "Q"
        elif self.value == 13:
            text = "K"
        elif self.value == 14:
            text = "A"
        else:
            text = str(self.value)
        if self.symbol == 0:    #D-Diamonds
            text += "D" 
        elif self.symbol == 1:  #H-Hearts
            text += "H"
        elif self.symbol == 2:  #S-Spade
            text += "S"
        else:   #C-Clubs
            text += "C" 
        return text   

    def getSymbol(self):
        return self.symbol

    def getValue(self):
        return self.value
    
    def lessThan(self, card):
        v1 = self.value + self.symbol * 15
        v2 = card.getValue() + card.getSymbol() * 15
        return v1 < v2
    
def maxCard(comb):
    values = []
    for c in comb:
        val = c.getValue() + 15 * c.getSymbol()
        values.append((val, c))
    maxValue, card = max(values)
    return card

# this class defines a deck of cards and its methods     
class deck:
    #Initializes the deck, and adds jokers if specified
    def __init__(self):
        self.cards = []
        self.inplay = []
        for symbol in range(0,4):
            for value in range (2,15):
                self.cards.append( Card(symbol, value) )

    #Shuffles the deck
    def shuffle(self):
        self.cards.extend( self.inplay )
        self.inplay = []
        shuffle( self.cards )
    
    #Cuts the deck by the amount specified
    #Returns true if the deck was cut successfully and false otherwise
    def cut(self, amount):
        if not amount or amount < 0 or amount >= len(self.cards):
            return False #returns false if cutting by a negative number or more cards than in the deck
        temp = [] 
        for i in range(0,amount):
            temp.append( self.cards.pop(0) )
        self.cards.extend(temp)
        return True

    #Returns a data dictionary 
    def deal(self, number_of_cards):
        if(number_of_cards > len(self.cards) ):
            return False #Returns false if there are insufficient cards
        inplay = []
        for i in range(0, number_of_cards):
            inplay.append( self.cards.pop(0) )
        self.inplay.extend(inplay)            
        return inplay      
    
    def cards_left(self):
        return len(self.cards)