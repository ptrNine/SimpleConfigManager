#include <string>
#include <array>
#include <vector>
#include <iostream>
#include <scm/scm.hpp>

class Ammo {
public:
    Ammo(const std::string_view& section) {
        auto scm_current_sect = section;

        SCM_MSET_CUR(pack_size);
        SCM_MSET_CUR(tracer);
        SCM_MSET_CUR(buck_shot);
        SCM_MSET_IKE_CUR(ap_factor, 1.0);
    }

    friend std::ostream& operator<< (std::ostream& os, const Ammo& ammo) {
        os << "\tPack size: " << ammo._pack_size << std::endl;
        os << "\tTracer: " << std::boolalpha << ammo._tracer << std::endl;
        os << "\tBuck shot: " << ammo._buck_shot << std::endl;
        os << "\tAp factor: " << ammo._ap_factor;

        return os;
    }

private:
    unsigned _pack_size;
    bool _tracer;
    unsigned _buck_shot;
    double _ap_factor;
};

class Weapon {
public:
    Weapon(const std::string_view& section) {
        _name = scm::read<std::string>("name", section);

        // But this way better
        scm::set(_mag_size, "mag_size", section);

        // Or this
        SCM_MSET(ammo_sections, section);

        // :)
        auto scm_current_sect = section;

        SCM_MSET_CUR(hit_power);
        SCM_MSET_CUR(flame_bone);
        SCM_MSET_CUR(flame_pos);
    }

    friend std::ostream& operator<< (std::ostream& os, const Weapon& wpn) {
        os << "Name: " << wpn._name << std::endl;
        os << "MagSize: " << wpn._mag_size << std::endl;
        os << "Hit Power: " << wpn._hit_power << std::endl;
        os << "Flame bone: " << wpn._flame_bone << std::endl;
        os << "Flame position: { " << wpn._flame_pos[0] << ", "
                                   << wpn._flame_pos[1] << ", "
                                   << wpn._flame_pos[2] << "}" << std::endl;

        os << "Ammo sections: " << std::endl;
        for (auto& s : wpn._ammo_sections) {
            os << "Section [" << s << "]:" << std::endl;
            os << Ammo(s) << std::endl;
        }

        return os;
    }


private:
    std::string _name;
    unsigned _mag_size;
    std::vector<std::string> _ammo_sections;
    double _hit_power;
    std::string _flame_bone;
    std::array<double, 3> _flame_pos;
};


int main() {
    scm::parse(scm::fs::default_cfg_path());

    std::cout << Weapon("weapon_ak228") << std::endl;
}