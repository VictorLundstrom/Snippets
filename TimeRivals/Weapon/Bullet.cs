using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Bullet : MonoBehaviour
{
    private string[] _bulletHit = new string[2] { "event:/Master/Weapons/Bow/Arrow Hit 1", "event:/Master/Weapons/Bow/Arrow Hit 2" };
    [SerializeField] private int _bulletLifetime;
    [SerializeField] private int _damage;
    [SerializeField] private int _impactForce;

    private int _playerID = -1;
    public int PlayerID { get { return _playerID; } set { _playerID = value; } }

    private bool _hasHit; //used to prevent multiple hits
    private void Awake()
    {
        Destroy(gameObject, _bulletLifetime);
    }
    private void OnCollisionEnter(Collision collision)
    {
        if (_hasHit)
        {
            return;
        }

        _hasHit = true;
   
        Destroy(gameObject);

        int randNr = Random.Range(0, _bulletHit.Length);
        FMODUnity.RuntimeManager.PlayOneShot(_bulletHit[randNr]);

        if (collision.gameObject.layer == LayerMask.NameToLayer("Default") || collision.gameObject.layer == LayerMask.NameToLayer("Saw")) //If Bullet Collided with the environment or Saw
        {
            return;
        }

        if (collision.gameObject.layer == LayerMask.NameToLayer("PhysicalObject")) //If we hit a physical object in the world
        {
            collision.rigidbody.AddForce(transform.forward * (_impactForce / 5), ForceMode.Impulse);
            return;
        }
        collision.rigidbody.AddForce(transform.forward * _impactForce, ForceMode.Impulse);

        GameObject targetPlayer = collision.transform.root.GetChild(0).gameObject; //if we hit a player
        targetPlayer.GetComponent<HealthSystem>().Damage(_damage, _playerID);

    }
}


