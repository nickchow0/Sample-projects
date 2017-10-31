import deck, collections, copy, cPickle, csv, random, scoring, sys, time
import numpy as np
from ast import literal_eval

def avg(L):
    total = (sum(L) + 0.0) / len(L)
    return total
 
# class that defines a poker player
class AIPlayer:
    #role \in \{small blind, big blind\}
    def __init__(self, cash, numberOfSamples, thresh, nudge = 2.0, thresh1 = 0.45, thresh2 = 0.55):        
        self.cash = cash     
        self.depth = 1
        self.playerType = "AI"
        self.NUM_SAMPLES = numberOfSamples
        self.THRESH = thresh
        self.nudge= nudge
        self.thresh1 = thresh1
        self.thresh2 = thresh2
        with open('AI_turn.pkl', 'rb') as fid:
            self.f_turn = cPickle.load(fid) 
        self.f_turn.set_params(verbose = 0)           
        with open('AI_river.pkl', 'rb') as fid:
            self.f_river = cPickle.load(fid) 
        self.f_river.set_params(verbose = 0)  
        self.preflopprobs = {}
        preflopcounts = {}
        preflopcounts = collections.defaultdict(int)
        self.preflopprobs = collections.defaultdict(float)
        with open('preflop_probs_100k_v2.csv', 'rb') as csvfile:
            spamreader = csv.reader(csvfile, delimiter=',', quotechar='"')
            for row in spamreader:                
                if row[0][0] == "(":
                    card1 = (literal_eval(row[0]))
                    card2 = (literal_eval(row[1]))
                    if card1[0] == card2[0]:
                        suit = 1
                    else:
                        suit = 0
                    large = max(card1[1], card2[1])
                    small = min(card1[1], card2[1])
                    self.preflopprobs[(large, small, suit)] += literal_eval(row[3])
                    preflopcounts[(large, small, suit)] += 1
        for key in self.preflopprobs:
            self.preflopprobs[key] = (self.preflopprobs[key] + 0.0) / preflopcounts[key]
        
    # approximate value of a state using a model built from random forest
    def evaluationFunction(self, state):
        if state.stage == 2:
            return self.estimatedVal(state, 2)
        if state.stage == 3:
            return self.estimatedVal(state, 3)
        if state.stage == 4:
            cp = copy.deepcopy(state)
            results = cp.determine_score(cp.communityCards, [cp.playerCards, cp.oppCards])
            cp.results = results
            winner = cp.determine_winner(results)                
            if winner == 0:
                return cp.totalBet              
            else:
                return 0
        print state.stage
        raise Exception("Evaluation Function Called in the Wrong Place")
        
    def preFlopStrategy(self, gameState):
        card1 = gameState.playerCards[0]
        card2 = gameState.playerCards[1]
        if card1.symbol == card2.symbol:
            suit = 1
        else:
            suit = 0
        large = max(card1.value, card2.value)
        small = min(card1.value, card2.value)
        prob = self.preflopprobs[(large, small, suit)]
        print prob
        actions = gameState.getLegalMoves()
        if prob < self.thresh1:
            return 2
        if prob > self.thresh2 and 5 in actions:
            return 5
        actions.remove(2)
        return random.choice(actions)
            
    # receives a gameState. Based on the gameState, it returns an action.
    # action is a subset of # 0=1/2 bet, 1=full bet, 2=fold, 3=check, 4= call, 5=raise, 6=showdown
    def bet(self, gameState):
        text = "Players hand: "
        for card in self.cards:
            text += str(card) + "  "
        print text  
        print "-----------------------"
        
        actions = gameState.getLegalMoves()
        print "List of Legal Moves: "
        print gameState.getLegalMoves()
        
        if len(actions) == 0:
            raise Exception("No Actions Available")
        if len(actions) == 1:
            output = actions[0]
        elif gameState.stage == 0:
            output = self.preFlopStrategy(gameState)
        else:
            choices = []
            for action in actions:               
                allPossibleSuccessors = gameState.get_possible_successors(action, 0)                
                cost = gameState.actionCost(action)
                total = 0
                LENGTH = min(self.THRESH[gameState.stage], len(allPossibleSuccessors))
                allPossibleSuccessors = random.sample(allPossibleSuccessors, LENGTH) 
                add = 0
                if action != 2:
                    add = self.nudge
                    #print add
                for successor in allPossibleSuccessors:        
                    successor.deck.cards += successor.oppCards
                    successor.oppCards = []
                    possibleOppCards = successor.giveAllPossibilities(2)                                   
                    possibleOppCards = random.sample(possibleOppCards, self.NUM_SAMPLES)                                            
                    for oppCard in possibleOppCards:
                        successor.deck.cards += successor.oppCards
                        successor.oppCards = [] 
                        successor.setPlayerCards(oppCard, 1)                                                    
                        total += self.evaluate(successor, self.depth, "", 0) - cost                                                                                                      
                choices.append((total / (0.0 + len(allPossibleSuccessors) * self.NUM_SAMPLES) + add, action))            
            bestIndices = [index for index in range(len(choices)) if choices[index][0] == max(choices)[0]]
            chosenIndex = random.choice(bestIndices)
            output = choices[chosenIndex][1]
            print "Values were"
            print choices
            print "My Minimax Expected Future gain is %f" %(max(choices)[0])
            print "My Minimax Expected Win is %f" %(max(choices)[0] - gameState.costs)
        print "My Minimax Cost so far  is %f" %(gameState.costs)
        print "My chosen action is %d" %output     
        return output           
        
    def estimatedVal(self, state, ind):
        potvalue = state.totalBet    
        if ind == 2:
            matrix = [0] * 164            
            player_win = 0
            opp_win = 0
            tie = 0
            total = 0
            temp = state.giveAllPossibilities(1)
            assert len(state.communityCards) == 4
            score = copy.deepcopy(state)
            for river in temp:
                community_cards = [score.communityCards[0], score.communityCards[1], score.communityCards[2], score.communityCards[3]]+ river
                players_hands = [[score.playerCards[0], score.playerCards[1]], [score.oppCards[0], score.oppCards[1]]]
                
                final_scores = score.determine_score(community_cards, players_hands)
                winner = score.determine_winner(final_scores)
                if type(winner) == list:
                    tie += 1
                    total += 1
                if winner == 0:
                    player_win += 1
                    total += 1
                if winner == 1:
                    opp_win += 1
                    total += 1
            total = total + 0.0
            l2 = len(state.playerCards)
            assert 2 ==l2
            assert len(state.communityCards) == 4
            # ENCODE PART:            
           
            for card in state.playerCards:
                val = card.symbol * 13 + (card.value - 2)  
                matrix[val] += 1
            
            for card in state.oppCards:
                val = card.symbol * 13 + (card.value - 2)  + 52
                matrix[val] += 1
            
            for card in state.communityCards:
                val = card.symbol * 13 + (card.value - 2)  + 104
                matrix[val] += 1

            if potvalue == 10:
                matrix[156] = 1
            if potvalue == 20:
                matrix[157] = 1
            if potvalue == 30:
                matrix[158] = 1
            if potvalue == 40:
                matrix[159] = 1
            if potvalue == 50:
                matrix[160] = 1
            if state.firstPlayer == 0:
                matrix[161] = 1
       		#probability of player winning (feat. 260)
            matrix[162] = float(player_win/total)
       		#probability of opponent winning (feat. 261)
            matrix[163] = float(tie/total)
      		#probability of tie (feat. 262)
            X = np.array(matrix)
            X = X.reshape(1, -1)
            pred = self.f_turn.predict(X)
            return pred
        else:
            matrix = [0] * 399            
            player_card1, player_card2 = state.playerCards
            opp_card1, opp_card2 = state.oppCards
            comm_card1, comm_card2, comm_card3, comm_card4, comm_card5 = state.communityCards
            player_win = 0
            opp_win = 0
            tie = 0
            players_hands = [[player_card1, player_card2], [opp_card1, opp_card2]]
            community_cards = [comm_card1, comm_card2, comm_card3, comm_card4, comm_card5]
            final_scores = state.determine_score(community_cards, players_hands)
            winner = state.determine_winner(final_scores)
            if type(winner) == list:
               	tie += 1
            if winner == 0:
                player_win += 1
           	if winner == 1:
           		opp_win += 1
            # ENCODE PART:            
            for card in state.playerCards: #(feat. 0-51)
                val = card.symbol * 13 + (card.value - 2)  
                matrix[val] += 1
            for card in state.oppCards: #(feat. 52-103)
                val = card.symbol * 13 + (card.value - 2)  + 52
                matrix[val] += 1
            for card in state.communityCards: #(feat. 104-155)
                val = card.symbol * 13 + (card.value - 2)  + 104
                matrix[val] += 1
            if potvalue == 10:
                matrix[156] = 1
            if potvalue == 20:
                matrix[157] = 1
            if potvalue == 30:
                matrix[158] = 1
            if potvalue == 40:
                matrix[159] = 1
            if potvalue == 50:
                matrix[160] = 1
            if potvalue == 60:
            	matrix[161] = 1
            if potvalue == 70:
            	matrix[162] = 1
            if state.firstPlayer == 0:
                matrix[163] = 1
        	#probability of player winning
            matrix[164] = player_win
        	#probability of opponent winning
            matrix[165] = tie
            #other potential features
            shift = 166
            #looking only at player cards
            #two pair...ace pair
            if player_card1.value == player_card2.value:
            	matrix[player_card1.value - 2+shift] = 1
            #one away...five away
            if abs(player_card1.value - player_card2.value) <= 5 and player_card1.value != player_card2.value:
            	x = abs(player_card1.value - player_card2.value)
            	matrix[12 + x + shift] = 1
            #suited diamonds...suited spades (feat. 18-21)
            if player_card1.symbol == player_card2.symbol:
	            matrix[18 + player_card1.symbol + shift] = 1            
            #cards add up to 4 (two twos),...cards add up to 28 (two aces) (feat. 22 - 46)
            x = player_card1.value + player_card2.value
            matrix[21+x-3+shift] = 1
            #suited AND one away...suited AND five away (feat. 47-51)
            if player_card1.symbol == player_card2.symbol and abs(player_card1.value - player_card2.value) <= 5 and player_card1.value != player_card2.value:
           		matrix[47 + player_card1.symbol+shift] = 1
            #suited AND add up to 4 (two twos - should not exist)...suited AND add up to 28 (two aces - should not exist) (feat. 52 - 76)
            if player_card1.symbol == player_card2.symbol:
           		x = player_card1.value + player_card2.value
           		matrix[51+x-3+shift] = 1           	
           	#looking only at opponent cards
           	#two pair...ace pair (feat. 77-89)
            if opp_card1.value == opp_card2.value:
           		matrix[77+opp_card1.value-2+shift] = 1 
            #one away...five away (feat. 90-94)
            if abs(opp_card1.value - opp_card2.value) <= 5 and opp_card1.value != opp_card2.value:
           		x = abs(opp_card1.value - opp_card2.value)
           		matrix[89+x+shift] = 1
            #suited diamonds...suited spades (feat. 95-98)
            if opp_card1.symbol == opp_card2.symbol:
           		matrix[95+opp_card1.symbol+shift] = 1
            #cards add up to 4 (two twos),...cards add up to 28 (two aces) (feat. 99 - 123)
            x = opp_card1.value + opp_card2.value
            matrix[98+x-3+shift] = 1
            #suited AND one away...suited AND five away (feat. 124-128)
            if opp_card1.symbol == opp_card2.symbol and abs(opp_card1.value - opp_card2.value) <= 5 and opp_card1.value != opp_card2.value:
           		matrix[124+opp_card1.symbol+shift] = 1
            #suited AND add up to 4 (two twos - should not exist)...suited AND add up to 28 (two aces - should not exist) (feat. 129 - 153)
            if opp_card1.symbol == opp_card2.symbol:
           		x = opp_card1.value + opp_card2.value
           		matrix[128+x-3+shift] = 1            
            #looking only at community cards
            #cards add up to 11 (three twos)...cards add up to 69(three aces) (feat. 154 - 212)
            x = comm_card1.value + comm_card2.value + comm_card3.value + comm_card4.value + comm_card5.value
            matrix[154+x-11+shift] = 1        
            #looking only at player + community cards
            #check score (defined in scoring) == 0 - 9 (feat. 213 - 222)
            hand = [player_card1, player_card2, comm_card1, comm_card2, comm_card3, comm_card4, comm_card5]
            x = state.score(hand)[0]
            matrix[213+x+shift] = 1   
            #looking only at opponent + community cards
            #check score (defined in scoring) == 0 - 9 (feat 223 - 232)
            hand = [opp_card1, opp_card2, comm_card1, comm_card2, comm_card3, comm_card4, comm_card5]
            x = state.score(hand)[0]
            matrix[223+x+shift] = 1
            X = np.array(matrix)
            X = X.reshape(1, -1)
            pred = self.f_river.predict(X)
            return pred
        
    def evaluate(self, state, depth, txt, playerType):
        debug = 0
        if state.isWin == 1 or state.isWin == -1:                
            return state.get_score()
        if depth == 0:
            return self.evaluationFunction(state)
        if depth < 0 :
            raise Exception('Depth Missed')
        if playerType == 1:
            if state.stage == 1:                
                allPossibleCommCards = state.giveAllPossibilities(3)
            else:
                allPossibleCommCards = state.giveAllPossibilities(1)        
            values = []        
            allPossibleCommCards = random.sample(allPossibleCommCards, self.THRESH[state.stage - 1])
            for commCards in allPossibleCommCards:
                state.setCommunityCards(commCards)
                val = self.evaluate(state, depth - 1, txt + "   ", 0)
                values.append(val)
                state.removeCommunityCards(commCards)        
            return avg(values)
        else:                                  
            currStage = state.stage 
            legalActions = state.getLegalMoves()                
            rewardList = []                                  
            if debug == 1:
                print txt + "Player %d: "%state.playerTurn
                print txt + str(legalActions)                
            if state.playerTurn == 0:      
                if len(state.oppCards) == 2:                
                    realCards = [state.oppCards[0], state.oppCards[1]]   
                else:
                    raise Exception("State should be complete")
                count = 0                                                     
                for action in legalActions:
                    s, possibilities = state.get_possible_options(action, 0)                                            
                    s.deck.cards += s.oppCards
                    s.oppCards = []
                    possibleOppCards = s.giveAllPossibilities(2)                                
                    if debug == 1:
                        print txt + "Action Taken: "
                        print txt + str(action)                    
                    
                    cost = state.actionCost(action)                        
                    newStage = s.stage
                    diff = newStage - currStage                 
                    possibleOppCards = random.sample(possibleOppCards, self.NUM_SAMPLES)                        
                    possibleRewards = []
                    for oppCard in possibleOppCards:
                        s.deck.cards += s.oppCards
                        s.oppCards = [] 
                        s.setPlayerCards(oppCard, 1)                            
                        if len(possibilities) == 0:                                    
                            reward = self.evaluate(s, depth - diff, txt + "   ", 0) - cost                                                        
                        else:
                            reward = self.evaluate(s, depth, txt + "   ", 1) - cost                            
                        possibleRewards.append(reward)                    
                    actionValue = avg(possibleRewards)
                    rewardList.append((actionValue, count))
                    count += 1                    
                # ind is the chosen action
                Expval, ind = max(rewardList)
                action = legalActions[ind]
                cost = state.actionCost(action)     
                s, possibilities = state.get_possible_options(action, 0)  
                s.deck.cards += s.oppCards
                s.oppCards = []
                s.setPlayerCards(realCards, 1)
                newStage = s.stage
                diff = newStage - currStage 
                if len(possibilities) == 0:
                    reward = self.evaluate(s, depth - diff, txt + "   ", 0) - cost
                else:
                    reward = self.evaluate(s, depth, txt + "   ", 1) - cost
                return reward
            if state.playerTurn == 1:                                
                for action in legalActions:
                    s, possibilities = state.get_possible_options(action, 1)                                        
                    if debug == 1:
                        print txt + "Action Taken: "
                        print txt + str(action)                        
                        sys.stdout.flush()
                    newStage = s.stage
                    diff = newStage - currStage 
                    if len(possibilities) == 0:
                        reward = self.evaluate(s, depth - diff, txt + "   ", 0)
                    else:
                        reward = self.evaluate(s, depth, txt + "   ", 1)
                    rewardList.append(reward)                        
                return min(rewardList)
