import random

from Services import servicesGlobalVariables as globalVar
from Services.Service_Game_Data import building_dico, road_dico, removing_cost
from Services.servicesmMapSpriteToFile import water_structures_types, farm_types
from CoreModules.GameManagement import Update as updates
from CoreModules.BuildingsManagement import buildingsManagementBuilding as buildings
from CoreModules.WalkersManagement import walkersManagementWalker as walkers

import copy

INIT_MONEY = 100000000
TIME_BEFORE_REMOVING_DWELL = 3 #seconds
import time

"""
ATTENTION: Building en feu pourrait etre updated
"""
class Game:
    def __init__(self, _map, name="save", game_view = None):
        self.name = name
        self.map = _map
        self.startGame()
        self.scaling = 0

        self.money = INIT_MONEY
        self.food = 0
        self.potery = 0
        self.likeability = 0
        self.gods_favors = [0, 0, 0, 0, 0]
        self.caesar_score = 0
        self.unemployement = 0
        self.isPaused = False
        self.buildinglist = []
        self.walkersAll = []
        self.walkersOut = []
        self.unemployedCitizens = []

        self.framerate = globalVar.DEFAULT_FPS
        self.updated = []

        # some lists of specific buildings
        self.dwelling_list = []

        self.water_structures_list = []
        self.food_structures_list = []
        self.temple_structures_list = []
        self.education_structures_list = []
        self.fountain_structures_list = []
        self.basic_entertainment_structures_list = []
        self.pottery_structures_list = []
        self.bathhouse_structures_list = []

        self.granary_list = [ ]

        self.reservoir_list = []

        #
        self.last_water_structure_removed = None
        self.last_food_structure_removed = None
        self.last_temple_structure_removed = None
        self.last_education_structure_removed = None
        self.last_fountain_structure_removed = None
        self.last_basic_entertainment_structure_removed = None
        self.last_pottery_structure_removed = None
        self.last_bathhouse_structure_removed = None
        self.last_reservoir_removed = None

        # Timer
        self.init_time = time.time()
        self.timer_for_immigrant_arrival = {}
        self.tmp_ref_time = time.time()
        # a dic of timers to track dwells with no roads
        # it associates each position of dwell with a timer
        self.timer_track_dwells = {}

        self.paths_for_buildings = { }

    def startGame(self):
        # ---------------------------------#
        _path = self.map.path_entry_to_exit(self.map.roads_layer.get_entry_position(),
                                            self.map.roads_layer.get_exit_position() )
        self.map.roads_layer.build_path_entry_to_exit(_path, [self.map.hills_layer, self.map.trees_layer])

    def change_game_speed(self, step):
        """
        A step of 1 indicates incremental speed
        And -1 indicates decremental speed
        """
        if self.framerate > globalVar.DEFAULT_FPS:
            if step == 1 and self.framerate < 10*globalVar.DEFAULT_FPS:
                self.framerate += globalVar.DEFAULT_FPS
            elif step == -1:
                self.framerate -= globalVar.DEFAULT_FPS
        elif self.framerate < globalVar.DEFAULT_FPS:
            if step == 1:
                self.framerate += 0.1*globalVar.DEFAULT_FPS
            elif step == -1 and self.framerate > 0.1*globalVar.DEFAULT_FPS:
                self.framerate -= 0.1*globalVar.DEFAULT_FPS
        else:
            if step == 1:
                self.framerate += globalVar.DEFAULT_FPS
            elif step == -1:
                self.framerate -= 0.1*globalVar.DEFAULT_FPS

    def foodproduction(self):
        # ---------------------------------#
        pass

    def updatebuilding(self, building: buildings.Building):
        current_state = (building.isBurning, building.isDestroyed,building.risk_level_dico["fire"],building.risk_level_dico["collapse"])
        if not building.isDestroyed:
            building.update_risk("fire")
            building.update_risk("collapse")
        updated_state = (building.isBurning, building.isDestroyed,building.risk_level_dico["fire"],building.risk_level_dico["collapse"])
        dico_change = {"fire": current_state[0] != updated_state[0],
                       "collapse" : current_state[1] != updated_state[1],
                       "fire_level" : (current_state[2] != updated_state[2],building.risk_level_dico["fire"]),
                       "collapse_level"  : (current_state[3] != updated_state[3],building.risk_level_dico["collapse"])
                      }
        return  dico_change

    def updateReligion(self):
        pass


    def print_building_list(self):
        for b in self.buildinglist:
            print(b.dic['version'])
    def update_supply_requirements(self, of_what: 'water' or 'food' or 'temple' or 'education' or 'fountain' or
                                         'basic_entertainment' or 'pottery' or 'bathhouse'):
        """
        This functions searches for supply structures on the map and for each one look for dwell within the range of
        the structure. If the dwell required a structure of this type, then its position will be added to the list of
        buildings to update.
        return: a set of positions of housings that will be updated, to avoid duplicate values
        """

        buildings_position_to_append_to_update_object = []
        tmp = of_what+'_structures_list'
        structures_list = getattr(self, tmp)
        for structure in structures_list:
            if not structure.is_functional():
                continue
            buildings_position_to_append_to_update_object += list(self.intermediate_update_supply_function(of_what,
                                                                                        structure, evolvable=True))
        return set(buildings_position_to_append_to_update_object)

    def intermediate_update_supply_function(self, of_what: 'water' or 'food' or 'temple' or 'education' or 'fountain' or
                                           'basic_entertainment' or 'pottery' or 'bathhouse', structure, evolvable=True):

        buildings_position_to_append_to_update_object = []
        if structure:  # None
            if structure.is_functional():
                _range = structure.range
                _position = structure.position

                for building in self.buildinglist:
                    line, column = building.position
                    if -_range + _position[0] <= line < _range + 1 + _position[0] and -_range + _position[1] <= column \
                            < _range + 1 + _position[1] and building.dic['version'] == "dwell":
                        building.update_requirements()
                        status = building.update_with_supply(of_what, evolvable=evolvable)
                        if status:
                            buildings_position_to_append_to_update_object.append((building.position,
                                                                                  building.structure_level))
        return set(buildings_position_to_append_to_update_object)

    def update_water_for_fountain_or_bath(self, reservoir, evolvable=True):
        if reservoir:  # None
            if reservoir.is_functional():
                _range = reservoir.range
                _position = reservoir.position

                for fountain_bath in self.water_structures_list:
                    if fountain_bath.dic['version'] in ["luxurious_bath", "normal_bath", "fountain", "fountain2",
                                                        "fountain3", "fountain4"]:
                        line, column = fountain_bath.position
                        if -_range + _position[0] <= line < _range + 1 + _position[0] and -_range + _position[1] <= column \
                                < _range + 1 + _position[1]:
                            fountain_bath.set_functional(evolvable)


    def downgrade_supply_requirement(self, of_what: 'water' or 'food' or 'temple' or 'education' or 'fountain' or
                                           'basic_entertainment' or 'pottery' or 'bathhouse'):
        tmp = 'last_'+of_what + '_structure_removed'
        structure = getattr(self, tmp)
        buildings_position_to_append_to_update_object = list(self.intermediate_update_supply_function(of_what, structure,
                                                                                                      evolvable=False))
        setattr(self, tmp, None)
        return buildings_position_to_append_to_update_object

    def updategame(self,scaling):
        """
        This function updates the game
        In fact it updates the buildings of the game but also the walkers
        Differents types of updates can occur: a building evolving, a building burning or a building collapsing
        """
        # The important object that will contain the updates
        self.scaling = scaling
        update = updates.LogicUpdate()

        # =======================================
        #  Updates of the walker
        # =======================================
        walker_to_update = set()
        for walker in self.walkersOut:
            if not walker.wait:
                status = walker.walk(self.scaling)
                if status == globalVar.IMMIGRANT_INSTALLED:
                    # An immigrant just set up --
                    new_status = walker.settle_in()
                    if new_status:
                        self.walkersAll.remove(walker)
                        self.walkersAll.append(new_status)
                        # we add a citizen as an unemployed
                        self.unemployedCitizens.append(new_status)

                elif status == globalVar.CITIZEN_IS_OUT:
                        walker_to_update.add(walker)

                elif status == globalVar.CITIZEN_ARRIVED:
                    if isinstance(walker,walkers.Prefect):
                        walker.instinguish_fire()
                        update.collapsed.append(walker.work_target.position)
                        walker.work_target = None

                    elif isinstance(walker, walkers.Cart_Pusher_Wheat):
                        granary = walker.transition_building
                        if granary:
                            walker.transition_building = None
                            granary.inc_storage()
                            walker.move_to_another_dwell(walker.work_building.position)
                            print(walker.work_building.position)
                            print(walker.current_path_to_follow)
                        else:
                            # arrived at it's house
                            walker.wait = True

                elif status is None:
                    if isinstance(walker, walkers.Citizen):
                        walker.work(self.get_buildings_for_walker(walker.init_pos), update)

        for w_to_update in walker_to_update:
            w_to_update.get_out_city()

        # =======================================
        #  Updates of the requirements
        # =======================================
        self.downgrade_supply_requirement('water')
        self.downgrade_supply_requirement('food')
        self.downgrade_supply_requirement('temple')
        self.downgrade_supply_requirement('education')
        self.downgrade_supply_requirement('fountain')
        self.downgrade_supply_requirement('basic_entertainment')
        self.downgrade_supply_requirement('pottery')
        self.downgrade_supply_requirement('bathhouse')

        self.update_supply_requirements('water')
        self.update_supply_requirements('food')
        self.update_supply_requirements('temple')
        self.update_supply_requirements('education')
        self.update_supply_requirements('fountain')
        self.update_supply_requirements('basic_entertainment')
        self.update_supply_requirements('pottery')
        self.update_supply_requirements('bathhouse')


        #-------------------------------------------------------------------------------------------------#
        # Updating of water structures wich functionality depends on a reservoir presence
        # Typically fountains
        # -------------------------------------------------------------------------------------------------#
        if self.last_reservoir_removed:
            self.update_water_for_fountain_or_bath(self.last_reservoir_removed, evolvable=False)

        for reservoir in self.reservoir_list:
            self.update_water_for_fountain_or_bath(reservoir)

        # Main loop that check each building built
        for k in self.buildinglist:
            # Some variables that will be used
            voisins = self.get_voisins_tuples(k.position)
            has_road = [self.map.roads_layer.is_real_road(v[0], v[1]) for v in voisins]
            possible_road = [v for v in voisins if self.map.roads_layer.is_real_road(v[0], v[1])]
            pos = k.position


            # Update of the risk speed level
            k.update_risk_speed_with_level()

            # Update of animation
            k.update_functional_building_animation()

            # Reset working building with no employee
            if k.need_employee and k.current_number_of_employees == 0 and k.is_functional():
                k.set_functional(False)

            # =======================================
            #  Creation of immigrants
            # =======================================
            if type(k) == buildings.Dwelling and not k.isDestroyed and not k.isBurning and \
                    k.current_number_of_employees < k.max_number_of_employees and any(has_road):
                if k not in self.paths_for_buildings.keys():
                    self.paths_for_buildings[k] = self.map.walk_to_a_building(self.map.roads_layer.get_entry_position(),
                                                                              None, k.position,[], walk_through=True)[1]
                path = self.paths_for_buildings[k]
                if path and (time.time()-self.tmp_ref_time) > 0.2:
                    self.tmp_ref_time = time.time()
                    # We call the required number of immigrants sequentially but with a delay of 2s to render
                    if k not in self.timer_for_immigrant_arrival.keys():
                        self.timer_for_immigrant_arrival[k] = time.time()
                    current_time = time.time()
                    if int( current_time - self.timer_for_immigrant_arrival[k]) > 0.5:
                        self.create_immigrant(path.copy(), k)
                        self.timer_for_immigrant_arrival[k] = current_time
                        # We remove the timer associated to this house if the max_population is reached
                        if k.current_number_of_employees == k.max_number_of_employees:
                            del self.timer_for_immigrant_arrival[k]
                            del self.paths_for_buildings[k]


            # =======================================
            #  Creation of prefects
            # =======================================
            elif k.dic['version'] == "prefecture"  and any(has_road) and \
                    k.current_number_of_employees < k.max_number_of_employees and not k.isDestroyed and not k.isBurning:
                if self.unemployedCitizens:
                    citizen = random.choice(self.unemployedCitizens)
                    prefet = citizen.change_profession("prefect")
                    self.walkersAll.remove(citizen)
                    self.unemployedCitizens.remove(citizen)

                    self.walkersAll.append(prefet)
                    self.walkersAll = list(set(self.walkersAll))

                    prefet.set_working_building(k)
                    k.add_employee(prefet.id, update_number=True)
                    if not k.is_functional():
                        k.set_functional(True)

                    prefet.init_pos = possible_road[2]
                    self.walkersGetOut(prefet)

            # =======================================
            #  Creation of engineers
            # =======================================
            elif k.dic['version'] == "engineer's_post" and any(has_road) and \
                    k.current_number_of_employees < k.max_number_of_employees and not k.isDestroyed and not k.isBurning:
                if self.unemployedCitizens:
                    citizen = random.choice(self.unemployedCitizens)
                    engineer = citizen.change_profession("engineer")
                    self.walkersAll.remove(citizen)
                    self.unemployedCitizens.remove(citizen)

                    self.walkersAll.append(engineer)
                    self.walkersAll = list(set(self.walkersAll))

                    engineer.set_working_building(k)
                    k.add_employee(engineer.id, update_number=True)
                    if not k.is_functional():
                        k.set_functional(True)

                    engineer.init_pos = possible_road[0]
                    self.walkersGetOut(engineer)
            # =======================================
            #  Creation of priest
            # =======================================
            elif k.dic['version'] in ["ares_temple","mars_temple","mercury_temple","neptune_temple","venus_temple"] and any(has_road) and \
                    k.current_number_of_employees < k.max_number_of_employees and not k.isDestroyed and not k.isBurning:
                if self.unemployedCitizens:
                    citizen = random.choice(self.unemployedCitizens)
                    priest = citizen.change_profession("priest")
                    self.walkersAll.remove(citizen)
                    self.unemployedCitizens.remove(citizen)

                    self.walkersAll.append(priest)
                    self.walkersAll = list(set(self.walkersAll))

                    priest.set_working_building(k)
                    k.add_employee(priest.id, update_number=True)
                    if not k.is_functional():
                        k.set_functional(True)

                    priest.init_pos = possible_road[2]
                    self.walkersGetOut(priest)

            # =======================================
            #  Creation of cart_pushers
            # =======================================
            elif k.dic['version'] in farm_types:
                if k.in_state_0():
                    #print("etat 0")
                    k.stop_production = True
                    if self.unemployedCitizens and any(has_road):

                        citizen = random.choice(self.unemployedCitizens)
                        ### pour toutes les fermes
                        pusher_wheat = citizen.change_profession("pusher_wheat")
                        self.walkersAll.remove(citizen)
                        self.unemployedCitizens.remove(citizen)

                        self.walkersAll.append(pusher_wheat)
                        self.walkersAll = list(set(self.walkersAll))

                        pusher_wheat.set_working_building(k)
                        print(pusher_wheat.work_building.position)
                        pusher_wheat.wait = True
                        self.walkersGetOut(pusher_wheat)

                        k.add_employee(pusher_wheat.id, update_number=True)
                        k.set_functional(True)
                        k.stop_production = False

                        i = random.randint(0, len(possible_road))
                        #print(f'{len(possible_road)} and {i}')
                        pusher_wheat.init_pos = possible_road[0]

                else:
                    pusher_id = list(k.get_all_employees())[0]
                    pusher = self.get_citizen_by_id(pusher_id)

                    if k.in_state_1():
                        #print("etat 1")
                        if self.unemployedCitizens and any(has_road):
                            citizen = random.choice(self.unemployedCitizens)
                            ### pour toutes les fermes
                            pusher_wheat = citizen.change_profession("pusher_wheat")
                            self.walkersAll.remove(citizen)
                            self.unemployedCitizens.remove(citizen)

                            self.walkersAll.append(pusher_wheat)
                            self.walkersAll = list(set(self.walkersAll))

                            pusher_wheat.set_working_building(k)
                            pusher_wheat.wait = True
                            k.add_employee(pusher_wheat.id, update_number=True)

                            pusher_wheat.init_pos = possible_road[0]
                            self.walkersGetOut(pusher_wheat)

                    elif k.in_state_2(pusher):
                        #print("etat 2")
                        k.stop_production = False

                    elif k.in_state_3(pusher):
                        #print("etat 3")
                        # Stop animation and production and the cart pusher working in it look for a granary
                        k.stop_production = True
                        find_granary = False
                        for granary in self.granary_list:
                            if granary.is_functional() and not granary.is_full():
                                if pusher.move_to_another_dwell(granary.position):
                                    find_granary = True
                                    pusher.transition_building = granary
                                    break
                        if find_granary:
                            pusher.wait = False
                            # print("reset anim etat2"+str(k.reset_animation))
                            k.reset_animation = True
                        else:
                            k.stop_production = False

                    elif k.in_state_4(pusher):
                        print("Etat4")
                        k.structure_level = 0
                        k.quantity = 0
                        k.stop_production = False
                        k.reset_animation = False

                    elif  k.in_state_5(pusher, self.get_voisins_tuples(k.position)):
                        # wait pusher
                        # pusher returned
                        pusher.wait = True


            # =======================================
            #  Granary management
            # =======================================
            elif k.dic['version'] == "granary" and any(has_road) and not k.isDestroyed and not k.isBurning and \
                k.current_number_of_employees < k.max_number_of_employees:
                if self.unemployedCitizens:
                    citizen = random.choice(self.unemployedCitizens)
                    self.unemployedCitizens.remove(citizen)
                    citizen.set_working_building(k)
                    k.add_employee(citizen.id, update_number=True)
                    if not k.is_functional():
                        k.set_functional(True)

            # =======================================
            #  Control of dwellings with no access to roads
            # =======================================
            if type(k) == buildings.Dwelling and not k.is_occupied():
                # we check if a road is next to this dwelling, if not we remove it after Xs
                removable = True
                for i, j in voisins:
                    if self.map.roads_layer.is_real_road(i, j):
                        removable = False
                        break
                # update tracktimer of dwells
                built_since = int(time.time() - self.timer_track_dwells[pos]) if pos in self.timer_track_dwells else 0

                if removable and built_since > TIME_BEFORE_REMOVING_DWELL:
                    # to avoid decreasing money
                    self.money += removing_cost
                    self.remove_element(pos)
                    update.removed.append(pos)
                    self.timer_track_dwells.pop(pos)

                elif built_since > TIME_BEFORE_REMOVING_DWELL:
                    self.timer_track_dwells.pop(pos)


            # =======================================
            #  Update of burnt and collapsed buildings
            # =======================================
            building_update = self.updatebuilding(k)
            cases = self.map.buildings_layer.get_all_positions_of_element(pos[0], pos[1])
            if building_update["fire"]:
                # the building is no more functional
                for i in cases:
                    k.functional = False
                    self.map.buildings_layer.array[i[0]][i[1]].isBurning = True
                    update.catchedfire.append(i)
                    
                    if type(k) == buildings.Dwelling:
                        self.guide_homeless_citizens(k)

            if building_update["collapse"]:
                # the building is no more functional
                for i in cases:
                    self.map.buildings_layer.array[i[0]][i[1]].isDestroyed = True
                    # the building is no more functional
                    k.functional = False
                    update.collapsed.append(i)
                    if type(k) == buildings.Dwelling:
                        self.guide_homeless_citizens(k)

            if building_update["fire_level"][0]:
                update.fire_level_change.append((k.position,building_update["fire_level"][1]))
            if building_update["collapse_level"][0]:
                update.collapse_level_change.append((k.position,building_update["collapse_level"][1]))

            prefets = self.get_prefets()
            buf = False
            if k.isBurning:
                for prefet in prefets:
                    if prefet.work_target and prefet.work_target == k:
                        buf = True
                        break
                if not buf:
                    for prefet in prefets:
                        if not prefet.work_target:
                            if prefet.move_to_another_dwell(k.position):
                                print(prefet.current_path_to_follow)
                                prefet.work_target = k
                            break

            # And a final update of all buildings
            update.has_evolved.append((k.position, k.structure_level))
        return update

    def get_citizen_by_id(self, id: int):
        for ctz in self.walkersAll:
            if ctz.id ==  id:
                return ctz

    def guide_homeless_citizens(self, building):
        """
        Note: Retire les citoyens même s'ils ne sont pas effectivement sortis
        """
        if type(building) == buildings.Dwelling:
            for ctz_id in building.get_all_employees():
                ctz = self.get_citizen_by_id(ctz_id)
                ctz.exit_way()
                if ctz in self.unemployedCitizens:
                    self.unemployedCitizens.remove(ctz)
                ctz.wait = False
                if ctz.work_building:
                    ctz.work_building.rem_employee(ctz_id)
                self.walkersGetOut(ctz)
            building.flush_employee()

        else:
            for ctz_id in building.get_all_employees():
                ctz = self.get_citizen_by_id(ctz_id)
                # Return in its house
                if ctz.move_to_another_dwell(ctz.house.position, walk_through=True):
                    self.walkersAll.remove(ctz)
                    if ctz in self.walkersOut:
                        self.walkersOut.remove(ctz)
                    ctz = ctz.change_profession("citizen")
                    self.walkersAll.append(ctz)
                    self.walkersAll = list(set(self.walkersAll))
                    self.unemployedCitizens.append(ctz)
                    ctz.work_building = None
                    ctz.wait = False
                    self.walkersGetOut(ctz)
                    break

            building.flush_employee()



    def get_buildings_for_walker(self, walker_position):
        voisins = self.get_voisins_tuples(walker_position)
        building_where_to_work = [b for b in self.buildinglist if b.position in voisins]
        return building_where_to_work


    def create_immigrant(self, path=[], building=None, display=False):
        walker = walkers.Immigrant(self.map.roads_layer.get_entry_position()[0],self.map.roads_layer.get_entry_position()[1],
                                   None, 0, self,path,building)
        self.walkersAll.append(walker)
        self.walkersAll = list(set(self.walkersAll))
        self.walkersGetOut(walker)

    def walkersGetOut(self, walker):
        self.walkersOut.append(walker)
        self.walkersOut = list(set(self.walkersOut))
        pass

    def walkersOutUpdates(self, exit=False):  # fps = self.framerate
        pass

    def remove_element(self, pos) -> str | None:
        """
        Cette fonction permet d'enlever un element de la map à une position donnée
        On ne peut pas retirer de l'herbe ou une montagne
        """
        if self.money < removing_cost:
            print("Not enough money")
            return None
        line, column = pos[0], pos[1]
        status, element_type, _element = self.map.remove_element_in_cell(line, column)
        if status:
            self.money -= removing_cost
            if element_type == globalVar.LAYER5:
                if self.buildinglist:
                    """
                    # to remove the time tracker if the building is removed before 3s
                    if pos in self.timer_track_dwells:
                        self.timer_track_dwells.pop(pos)
                    """
                    self.buildinglist.remove(_element)
                    self.guide_homeless_citizens(_element)
                    if type(_element) == buildings.Dwelling:
                        self.dwelling_list.remove(_element)

                    if type(_element) == buildings.WaterStructure:
                        # we must copy the element because if will potentially be  removed or changed in memory
                        self.last_water_structure_removed = copy.copy(_element)
                        self.water_structures_list.remove(_element)
                        if _element.dic['version'] == "reservoir":
                            self.last_reservoir_removed = copy.copy(_element)
                            self.reservoir_list.remove(_element)

                    if type(_element) == buildings.Granary:
                        self.granary_list.remove(_element)
                    del _element
        return element_type

    def remove_elements_serie(self, start_pos, end_pos) -> set:
        """
        Pour clean une surface de la carte
        Elle va renvoyer un ensemble set qui contient les layers qui ont été modifiés
        """
        line1, column1 = start_pos[0], start_pos[1]
        line2, column2 = end_pos[0], end_pos[1]

        # 2 ranges qui vont servir à délimiter la surface de la map à clean
        vrange, hrange = None, None

        # le set
        _set = set()

        if line1 >= line2:
            vrange = range(line1, line2 - 1, -1)
        else:
            vrange = range(line2, line1 - 1, -1)

        if column1 <= column2:
            hrange = range(column2, column1 - 1, -1)
        else:
            hrange = range(column1, column2 - 1, -1)

        for i in vrange:
            for j in hrange:
                result = self.remove_element((i, j))
                if result:
                    _set.add(result)
        return _set

    def add_road(self, line, column) -> bool:
        # Precondition: we must have enough money for adding a road
        if self.money < road_dico['cost']:
            print("Not enough money")
            return False
        status = self.map.roads_layer.set_cell_constrained_to_bottom_layer(self.map.collisions_layers, line, column)
        if status:
            self.money -= road_dico['cost']
        return status

    def add_roads_serie(self, start_pos, end_pos, dynamically=False) -> bool:
        # Here we can't precisely calculate the money that will be needed to construct all the roads. we'll estimate
        # that
        estimated_counter_roads = (abs(start_pos[0] - end_pos[0])) + (abs(start_pos[1] - end_pos[1])) + 1
        if self.money < estimated_counter_roads * road_dico['cost']:
            print("Not enough money")
            return False

        status, count = self.map.roads_layer.add_roads_serie(start_pos, end_pos,
                    self.map.collisions_layers, memorize=dynamically)

        if status and not dynamically:
            self.money -= road_dico['cost'] * count
        return status


    def add_building(self, line, column, version) -> bool:
        txt= " ".join(version.split("_"))
        if self.money < building_dico[txt].cost:
            print("Not enough money")
            return False
        # we have to determine the exact class of the building bcause they have not the same prototype
        if version == "dwell":
            building = buildings.Dwelling(self.map.buildings_layer, globalVar.LAYER5)
        elif version in water_structures_types:
            building = buildings.WaterStructure(self.map.buildings_layer, globalVar.LAYER5, version)
        elif version in farm_types:
            building = buildings.Farm(self.map.buildings_layer, globalVar.LAYER5, version)
        elif version == "granary":
            building = buildings.Granary(self.map.buildings_layer, globalVar.LAYER5)
        else:
            building = buildings.Building(self.map.buildings_layer, globalVar.LAYER5, version)

        # we should check that there is no water on the future positions
        cells_number = building.dic['cells_number']
        can_build_out_of_water = all([not self.map.grass_layer.cell_is_water(line + i, column + j)
                             for j in range(0, cells_number) for i in range(0, cells_number)])
        if not can_build_out_of_water:
            return False

        if version in farm_types:
            # we should check if there is yellow grass on the future positions to check
            can_build_on_yellow_grass = all([self.map.grass_layer.cell_is_yellow_grass(line + i, column + j)
                             for j in range(0, cells_number) for i in range(0, cells_number)])

            if not can_build_on_yellow_grass:
                return False

        status = self.map.buildings_layer.set_cell_constrained_to_bottom_layer(self.map.collisions_layers, line, column,
                                                                               building)
        if status:
            self.money -= building_dico[txt].cost
            self.buildinglist.append(building)
            if version == "dwell":
                # if user just built a dwell we associate a timer so that the dwell can be removed after x seconds
                self.timer_track_dwells[(line, column)] = time.time()
                self.dwelling_list.append(building)

            if type(building) == buildings.WaterStructure:
                self.water_structures_list.append(building)
                if building.dic['version'] == "reservoir":
                    self.reservoir_list.append(building)
                    # Functionality of a reservoir is calculated directly when it's created
                    # A reservoir is functional when adjacent to a river or a coast (water)
                    ajacent_to_water = any([self.map.grass_layer.cell_is_water(line + i, column + j)
                    for j in range(-1, cells_number+1) for i in range(-1, cells_number+1) if i in [-1, cells_number]
                                            or j in[-1, cells_number]])
                    if ajacent_to_water:
                        building.set_functional(True)

            if type(building) == buildings.Granary:
                self.granary_list.append(building)

        return status

    def update_likability(self,building):
        voisins = self.get_voisins(building)
        score = 0
        for voisin in voisins:
            version = voisin.dic["version"]
            if version not in ["null"]:
                if version == "dwell":
                    pass
                    

    def get_buildings_in_neighboorhood(self, pos):
        pass

    def get_voisins(self,building):
        voisins = set()
        cases = []
        pos = building.position
        for i in range(0, building.dic['cells_number']):
            for j in range(0, building.dic['cells_number']):
                if (i, j) != (0, 0):
                    cases.append(self.map.buildinglayer((pos[0] + i, pos[1] + j)))
        for case in cases:
            for i in range(-1,2):
                for j in range(-1,2):
                    voisins.add(self.map.buildinglayer.array[case[0] + i][case[1] + j])
        return voisins
    
    def get_voisins_tuples(self, pos):
        cases = []
        for i in range(-2,3):
            for j in range(-2,3):
                cases.append((pos[0] + i,pos[1] + j))
        return cases

    def get_prefets(self):
        prefets = []
        for walker in self.walkersOut:
            if isinstance(walker, walkers.Prefect):
                prefets.append(walker)
        return prefets
