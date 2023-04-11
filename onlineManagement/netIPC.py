import re 
import onlineManagement.netInterface as interNet

class netIPC():
	def __init__(self, ip="", port="", typeCo=1):
		super(netIPC, self).__init__()
		self.monPort = port
		self.monIP = ip
		self.net = interNet.NetInterface(1, ip, port)
		if typeCo==0:
			self.net.creat_srv()
		self.valid_ip = False
		self.valid_port = False
		self.isCheckOnes = False
		self.loading = False
		self.charge = 0.0
		self.buffer_s = []

	def ipChecker(self, monIP : str) -> bool:
		self.isCheckOnes = True
		reg = re.search("^[1-9][0-9]{1,2}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$",monIP)
		if reg == None:
			return False
		self.valid_ip = True
		return True

	def portCherker(self, monPort : str) -> bool:
		self.isCheckOnes = True
		reg = re.search("^[1-9][0-9]{3}[0-9]{0,1}$",monPort)
		if reg == None:
			return False
		return True if self.isValidPort(monPort) else False

	def isValidPort(self, num : str) -> bool:
		if int(num)>0 and int(num)<65000:
			self.valid_port = True
			return True
		return False

	def chaineSansBraket(self, chaine):
		maChaine = chaine[1:]
		return maChaine[:-1]

	def extractFrame(self, chaine : str) -> tuple:
		nb_open = 0
		nb_close = 0
		frame = ''
		for i in range(len(chaine)):
			frame += chaine[i]
			if chaine[i] == '{': nb_open += 1
			if chaine[i] == '}': nb_close += 1
			if nb_open == nb_close and nb_open > 0:
				if frame[0] == 'i': return (frame[2:-1], i, 0)
				else: return (frame[2:-1], i, 1)
		return ('', -1, -1)

	def decomposition(self, chaine, nbParties):
		dico = {}
		frame, index, t = self.extractFrame(chaine)
		if index > 0: self.net.translate_msg(index)
		dico[frame[0]] = frame[2:-1]
		return dico

	
	def bracketChecker(self, chaine):
		nbParties = 0
		nb_open = 0
		nb_close = 0
		for i in range(len(chaine)):
			if chaine[i] == "{":
				nb_open += 1
			if chaine[i] == "}":
				nb_close += 1
			if nb_open == nb_close and nb_open > 0:
				nbParties += 1
		if nbParties > 0:
			return nbParties
		return -1

	def extract_frame_send(self, chaine : str) -> tuple:
		acco_open = 0
		acco_close = 0
		index = 0
		for i in range(len(chaine)):
			if chaine[i] == '{': acco_open += 1
			if chaine[i] == '}': acco_close += 1
			if acco_open == acco_open and acco_open != 0:
				index = i
				break
		tmp = chaine[2:index+1]
		return (index, tmp)

	
	def protocolChecker(self, chaine):
		if len(chaine)  < 1:
			return -1
		chaine = chaine.decode()
		if chaine[0] == "i":
			nbParties = self.bracketChecker(chaine)
			if nbParties == -1:
				return -1
			parties = self.decomposition(chaine, nbParties)
			self.protocolAnalyser(parties)
			return None
		if chaine[0] == "s":
			recv = chaine[2:len(chaine)-1]
			print("reception de message pour le jeu : ", recv)
			self.buffer_s.append(recv)
			self.net.translate_msg(len(chaine))
			return None
		print("error protocolChecker")
		print(chaine)

	def protocolAnalyser(self, dico):
		for cle in [*dico]:
			match cle:
				case "r":
					self.arretProcessC(dico[cle])
				case "c":
					self.connexionUtilisateur(dico[cle])
				case "d":
					self.deconnexionUtilisateur(dico[cle])
				case "p":
					self.connectionProcessC(dico[cle])
				case "g":
					self.start_game(dico[cle])
				case "u":
					self.charge = int(dico[cle])
				case "a":
					self.change_id(dico[cle])
				case _:
					print("error protocolAnalyser")
	
	def connectionProcessC(self, infos):
		self.monPort = infos

	def arretProcessC(self, num):
		monNum = int(num)
		match monNum:
			case 0:
				print("Pas d'erreur")
			case 1:
				print("erreur argument")
			case 2:
				print("erreur init socket")
			case 3:
				print("pb écoute réseau")
			case 4:
				print("serveur stoppé")
			case _:
				print("error")

	def connexionUtilisateur(self, chaine, delim=","):
		tab = chaine.split(delim)
		monUser = tab[0]
		id_user = tab[1]
		listeUsers = self.net.getUsers()
		i = 0
		for user in listeUsers:
			if listeUsers[i]["id"] == -1:
				listeUsers[i]["id"] = int(id_user)
				listeUsers[i]["user"] = monUser
				self.net.setUsers(listeUsers)
				break
			i += 1
		
		return listeUsers

	def start_game(self, chaine):
		if(chaine == '0'):
			self.loading = True

	def change_id(self, chaine):
		self.net.users[0]['id'] = int(chaine)
	
	def deconnexionUtilisateur(self, chaine):
		if int(chaine) == 0:
			print("arret proc C")
		elif int(chaine):
			listeUsers = self.net.getUsers()
			i = 0
			monI = -1
			lastUser ={"user":"", "id":-1}
			lastUserId = -1
			for user in listeUsers:
				
				if int(listeUsers[i]["id"]) == int(chaine):
					monI = i
					print("i remplacement : ", i)
				if int(user["id"]) != -1 and int(user["id"]) != int(chaine) and int(user["id"]) != 0:
					lastUser = user
					lastUserId = i
					print("mon i : ", i)
				i += 1

			if monI != -1 and lastUserId != -1 and monI < lastUserId:
				#pas fini BEUG
				listeUsers[monI]["id"] = lastUser["id"]
				listeUsers[monI]["user"] = lastUser["user"]
				listeUsers[lastUserId]["user"] = ""
				listeUsers[lastUserId]["id"] = -1
			else:
				listeUsers[monI]["id"] = -1
				listeUsers[monI]["user"] = ""

			self.net.setUsers(listeUsers)

		else:
			print("erreur")

	def byteToText(self, text):
		return text.decode("utf-8") 

	def exec(self):
		if self.net.get_msg(): # si on a recu un nouveau message
			while self.protocolChecker(self.net.buf_recv) != -1:
				pass
