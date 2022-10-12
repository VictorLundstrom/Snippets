using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WeaponPickup : MonoBehaviour
{
    [SerializeField] private GameObject _weaponPrefab;

    [SerializeField] private AttackIndexes _weaponTypeIndex;
    [SerializeField] private WeaponSpawnManager _weaponSpawnManager;
    private bool _pickedUp;
    public bool PickedUp { get { return _pickedUp; } set { _pickedUp = value; } }

    private int _beingLootedByPlayerID = -1;
    public int BeingLootedByPlayerID { get { return _beingLootedByPlayerID; } set { _beingLootedByPlayerID = value; } }

    private const float _timeUntilDestroy = 5; //Extra leeway time so every Player's OnTriggerExit get's called BEFORE we destroy this object

    private int _weaponPickupID;
    public int WeaponPickupID { get { return _weaponPickupID; } set { _weaponPickupID = value; } }

    public GameObject WeaponPrefab { get { return _weaponPrefab; } }
    public AttackIndexes WeaponTypeIndex { get { return _weaponTypeIndex; } }

    //SFX
    private string[] _swordPickupSFX = new string[7] { "event:/Master/Weapons/Sword/Sword Sound 1", "event:/Master/Weapons/Sword/Sword Sound 2", "event:/Master/Weapons/Sword/Sword Sound 3", "event:/Master/Weapons/Sword/Sword Sound 4", "event:/Master/Weapons/Sword/Sword Sound 5", "event:/Master/Weapons/Sword/Sword Sound 6", "event:/Master/Weapons/Sword/Sword Sound 7" };

    private void Awake()
    {
        _weaponSpawnManager = WeaponSpawnManager.Instance;
        _weaponPickupID = WeaponSpawnManager.Instance.ActivePickupList.Count;
    }

    public void DestroyPickup()
    {
        _weaponSpawnManager.PickedupList.Remove(transform.position);

        switch (_weaponTypeIndex)
        {
            case AttackIndexes.SWORD:
                int randNr = Random.Range(0, _swordPickupSFX.Length);
                FMODUnity.RuntimeManager.PlayOneShot(_swordPickupSFX[randNr]);
                break;
            case AttackIndexes.BAT:
                FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Gun/Gun Shell drop");
                break;
            case AttackIndexes.RIFLE:
                FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Gun/Revolver Reload");
                break;
            case AttackIndexes.CROSSBOW:
                FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Bow/Bow string tension");
                break;
            case AttackIndexes.MALLET:
                FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Sword/Sword Swing Low");
                break;
            case AttackIndexes.MACEFISH:
                FMODUnity.RuntimeManager.PlayOneShot("event:/Master/Weapons/Sword/Sword Swing Low");
                break;
        }

        transform.position = new Vector3(10000.0f, 10000.0f, 10000.0f); //to trigger OnTriggerExit
        
        Destroy(gameObject, _timeUntilDestroy);
    }
}
